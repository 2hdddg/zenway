#include "Output.h"

#include "spdlog/spdlog.h"
#include "src/Registry.h"
#include "src/ShellSurface.h"

class Output {
    using OnNamedCallback = std::function<void(Output *output, const std::string &name)>;

   public:
    static void Create(wl_output *wloutput, const wl_output_listener *listener,
                       std::shared_ptr<Configuration> config, OnNamedCallback onNamed) {
        // This will be in lingo until name is received
        auto output = new Output(wloutput, config, onNamed);
        wl_output_add_listener(wloutput, listener, output);
    }

    void OnName(const char *name) {
        // This will only be invoked once per lifetime of output
        m_name = name;
        m_onNamed(this, m_name);
        m_onNamed = nullptr;
    }

    void OnDescription(const char * /*description*/) {
        // Might change over lifetime
    }

    virtual ~Output() {
        wl_output_destroy(m_wloutput);
        m_wloutput = nullptr;
    }

    void Draw(const Registry &registry, const PanelConfig &panelConfig, BufferPool &bufferPool) {
        // Query panel if it wants to be drawn on this display
        if (panelConfig.checkDisplay && !panelConfig.checkDisplay(m_name)) {
            return;
        }
        spdlog::info("Drawing panel {} on output {}", panelConfig.index, m_name);
        // Ensure that there is a surface for this panel
        if (!m_surfaces.contains(panelConfig.index)) {
            auto surface = ShellSurface::Create(registry, m_wloutput, panelConfig /* copies */);
            if (!surface) {
                spdlog::error("Failed to create surface");
                return;
            }
            m_surfaces[panelConfig.index] = std::move(surface);
        }
        m_surfaces[panelConfig.index]->Draw(registry, bufferPool, m_name);
    }

    void Hide(const Registry &registry) {
        for (const auto &kv : m_surfaces) {
            kv.second->Hide(registry);
        }
    }

    bool ClickSurface(wl_surface *surface, int x, int y) {
        for (auto &kv : m_surfaces) {
            if (kv.second->ClickSurface(surface, x, y)) {
                spdlog::debug("Clicked in surface");
                return true;
            }
        }
        spdlog::debug("No surface found for click");
        return false;
    }

   private:
    Output(wl_output *wloutput, std::shared_ptr<Configuration> config, OnNamedCallback onNamed)
        : m_wloutput(wloutput), m_config(config), m_onNamed(onNamed) {}

    std::map<int, std::unique_ptr<ShellSurface>> m_surfaces;  // Surface per panel index
    wl_output *m_wloutput;
    const std::shared_ptr<Configuration> m_config;
    // Temporary callback until named, registers amoung the other outputs when name received
    OnNamedCallback m_onNamed;
    std::string m_name;
};

static void on_name(void *data, struct wl_output *, const char *name) {
    ((Output *)data)->OnName(name);
}

void on_description(void *data, struct wl_output *, const char *description) {
    ((Output *)data)->OnDescription(description);
}

void on_geometry(void * /*data*/, struct wl_output *, int32_t /*x*/, int32_t /*y*/,
                 int32_t /*physical_width*/, int32_t /*physical_height*/, int32_t /*subpixel*/,
                 const char * /*make*/, const char * /*model*/, int32_t /*transform*/) {}

void on_mode(void * /*data*/, struct wl_output *, uint32_t /*flags*/, int32_t /*width*/,
             int32_t /*height*/, int32_t /*refresh*/) {}

void on_done(void * /*data*/, struct wl_output *) {}

void on_scale(void * /*data */, struct wl_output *, int32_t /*factor*/) {}

const struct wl_output_listener listener = {
    .geometry = on_geometry,
    .mode = on_mode,
    .done = on_done,
    .scale = on_scale,
    .name = on_name,
    .description = on_description,
};

std::unique_ptr<Outputs> Outputs::Create(std::shared_ptr<Configuration> config) {
    return std::unique_ptr<Outputs>(new Outputs(config));
}

bool Outputs::InitializeBuffers(wl_shm &shm) {
    m_bufferPool = BufferPool::Create(shm, m_config->numBuffers, m_config->bufferWidth,
                                      m_config->bufferHeight);
    if (!m_bufferPool) {
        spdlog::error("Failed to initialize buffer pool");
        return false;
    }
    return true;
}

void Outputs::Add(wl_output *wloutput) {
    Output::Create(wloutput, &listener, m_config, [this](auto output, auto name) {
        spdlog::info("Adding output {}", name);
        m_map[name] = std::shared_ptr<Output>(output);
    });
}

void Outputs::Draw(const Registry &registry, const Sources &sources) {
    spdlog::trace("Draw outputs");
    for (const auto &panelConfig : m_config->panels) {
        bool dirty = false;
        for (const auto &widgetConfig : panelConfig.widgets) {
            if (sources.NeedsRedraw(widgetConfig.sources)) {
                dirty = true;
                break;
            }
        }
        // This panel is dirty, redraw it on every output
        if (dirty) {
            for (const auto &keyValue : m_map) {
                keyValue.second->Draw(registry, panelConfig, *m_bufferPool);
            }
        }
    }
}

void Outputs::Hide(const Registry &registry) {
    for (auto &keyValue : m_map) {
        keyValue.second->Hide(registry);
    }
}

void Outputs::ClickSurface(wl_surface *surface, int x, int y) {
    for (auto &kv : m_map) {
        if (kv.second->ClickSurface(surface, x, y)) {
            return;
        }
    }
}
