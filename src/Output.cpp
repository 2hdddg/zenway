#include "Output.h"

#include "Roots.h"
#include "spdlog/spdlog.h"

static void on_name(void *data, struct wl_output *wl_output, const char *name) {
    ((Output *)data)->OnName(name);
}

void on_description(void *data, struct wl_output *wl_output, const char *description) {
    ((Output *)data)->OnDescription(description);
}

void on_geometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y,
                 int32_t physical_width, int32_t physical_height, int32_t subpixel,
                 const char *make, const char *model, int32_t transform) {}

void on_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height,
             int32_t refresh) {}

void on_done(void *data, struct wl_output *wl_output) {}

void on_scale(void *data, struct wl_output *wl_output, int32_t factor) {}

const struct wl_output_listener listener = {
    .geometry = on_geometry,
    .mode = on_mode,
    .done = on_done,
    .scale = on_scale,
    .name = on_name,
    .description = on_description,
};

void Output::Create(const std::shared_ptr<Roots> roots, wl_output *wloutput,
                    OnNamedCallback onNamed) {
    // This will be in lingo until name is received
    auto output = new Output(wloutput, onNamed);
    output->m_surfaces[0] = ShellSurface::Create(roots, wloutput);
    output->m_surfaces[1] = ShellSurface::Create(roots, wloutput);
    wl_output_add_listener(wloutput, &listener, output);
}

void Output::OnName(const char *name_) {
    // This will only be invoked once per lifetime of output
    name = name_;
    spdlog::trace("Event wl_output::name {}", name);
    m_onNamed(this);
    m_onNamed = nullptr;
}

void Output::OnDescription(const char *description) {
    // Might change over lifetime
    spdlog::trace("Event wl_output::description {}", description);
}

void Output::Draw(int panelId, Anchor anchor, Buffer &buffer, const Size &size) {
    m_surfaces[panelId]->Draw(anchor, buffer, size);
}

void Output::Hide() {
    for (const auto &surface : m_surfaces) {
        surface->Hide();
    }
}

std::unique_ptr<Outputs> Outputs::Create(const std::shared_ptr<Roots> roots) {
    return std::unique_ptr<Outputs>(new Outputs(roots));
}

void Outputs::Add(wl_output *wloutput) {
    Output::Create(m_roots, wloutput, [this](Output *output) {
        spdlog::debug("Found output {}", output->name);
        m_map[output->name] = std::shared_ptr<Output>(output);
    });
}

void Outputs::ForEach(std::function<void(std::shared_ptr<Output>)> callback) {
    for (auto &keyValue : m_map) {
        callback(keyValue.second);
    }
}
std::shared_ptr<Output> Outputs::Get(const std::string &name) const {
    auto keyValue = m_map.find(name);
    if (keyValue == m_map.end()) return nullptr;
    return keyValue->second;
}
