#include "ShellSurface.h"

#include <spdlog/spdlog.h>
#include <wayland-client-protocol.h>

#include "wlr-layer-shell-unstable-v1.h"

static void on_configure(void *data, struct zwlr_layer_surface_v1 *layer, uint32_t serial,
                         uint32_t cx, uint32_t cy) {
    auto shellSurface = (ShellSurface *)data;
    shellSurface->OnShellConfigure(cx, cy);
    zwlr_layer_surface_v1_ack_configure(layer, serial);
}

static void on_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1) {
    auto shellSurface = (ShellSurface *)data;
    shellSurface->OnClosed();
}

static void on_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {}

static const zwlr_layer_surface_v1_listener layer_listener = {.configure = on_configure,
                                                              .closed = on_closed};

static const wl_surface_listener surface_listener = {
    .enter = on_enter,
    .leave = nullptr,
};

std::unique_ptr<ShellSurface> ShellSurface::Create(const std::shared_ptr<Roots> roots,
                                                   wl_output *output,
                                                   PanelConfig panelConfig) {
    auto surface = wl_compositor_create_surface(roots->compositor);
    auto shellSurface = std::unique_ptr<ShellSurface>(
        new ShellSurface(roots, output, surface, std::move(panelConfig)));
    wl_surface_add_listener(surface, &surface_listener, shellSurface.get());
    wl_surface_commit(surface);
    return shellSurface;
}

void ShellSurface::OnShellConfigure(uint32_t cx, uint32_t cy) {
    spdlog::trace("Event zwlr_layer_surface::configure size {}x{}", cx, cy);
};

void ShellSurface::OnClosed() {
    Hide();
    // Should destroy this!
    m_isClosed = true;
}

bool ShellSurface::ClickSurface(wl_surface *surface, int x, int y) {
    bool isThis = surface == m_surface;
    if (!isThis) {
        return false;
    }

    // Check what widget
    int i = 0;
    for (const auto w : m_drawn.widgets) {
        if (w.position.Contains(x, y)) {
            spdlog::trace("Found widget click target");
            auto &widget = m_panelConfig.widgets.at(i);
            if (widget.click) {
                widget.click();
            }
            break;
        }
        i++;
    }
    // Return true even if no widget was found to stop trying other surfaces
    return true;
}

void ShellSurface::Show() {
    if (m_isClosed) return;
    if (m_layer) return;
    m_layer = zwlr_layer_shell_v1_get_layer_surface(m_roots->shell, m_surface, m_output,
                                                    ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "namespace");
    zwlr_layer_surface_v1_add_listener(m_layer, &layer_listener, this);
    zwlr_layer_surface_v1_set_size(m_layer, 1, 1);
    zwlr_layer_surface_v1_set_anchor(m_layer, ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    wl_surface_commit(m_surface);
    m_roots->FlushAndDispatchCommands();
}

void ShellSurface::Draw(BufferPool &bufferPool, const std::string &outputName) {
    if (m_isClosed) {
        return;
    }
    if (!m_layer) {
        Show();
    }
    m_drawn.widgets.clear();
    if (!Panel::Draw(m_panelConfig, outputName, bufferPool, m_drawn)) {
        // Nothing drawn for this output
        return;
    }
    const auto &size = m_drawn.size;
    Rect damage;
    damage.x = 0;
    damage.y = 0;
    damage.cx = std::max(size.cx, m_previousDamage.cx);
    damage.cy = std::max(size.cy, m_previousDamage.cy);
    spdlog::trace("Draw buffer: {}x{}", size.cx, size.cy);
    zwlr_layer_surface_v1_set_size(m_layer, size.cx, size.cy);
    auto anchor = m_panelConfig.anchor;
    uint32_t zanchor;
    switch (anchor) {
        case Anchor::Left:
            zanchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
            break;
        case Anchor::Right:
            zanchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
            break;
        case Anchor::Top:
            zanchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
            break;
        case Anchor::Bottom:
            zanchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
            break;
    }
    zwlr_layer_surface_v1_set_anchor(m_layer, zanchor);
    wl_surface_attach(m_surface, m_drawn.buffer->Lock(), 0, 0);
    wl_surface_damage_buffer(m_surface, damage.x, damage.y, damage.cx, damage.cy);
    // Maintain input region
    if (m_inputRegion) {
        wl_region_destroy(m_inputRegion);
    }
    m_inputRegion = wl_compositor_create_region(m_roots->compositor);
    wl_region_add(m_inputRegion, 0, 0, size.cx, size.cy);
    wl_surface_set_input_region(m_surface, m_inputRegion);
    // Commit changes
    wl_surface_commit(m_surface);
    m_roots->FlushAndDispatchCommands();
    m_previousDamage = {0, 0, size.cx, size.cy};
}

void ShellSurface::Hide() {
    if (!m_layer) return;
    zwlr_layer_surface_v1_destroy(m_layer);
    wl_surface_attach(m_surface, NULL, 0, 0);
    m_layer = nullptr;
    wl_region_destroy(m_inputRegion);
    m_inputRegion = nullptr;
    m_roots->FlushAndDispatchCommands();
}
