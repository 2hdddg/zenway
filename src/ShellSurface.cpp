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
                                                   wl_output *output, const std::string &name) {
    auto surface = wl_compositor_create_surface(roots->compositor);
    auto shellSurface =
        std::unique_ptr<ShellSurface>(new ShellSurface(roots, output, surface, name));
    wl_surface_add_listener(surface, &surface_listener, shellSurface.get());
    wl_surface_commit(surface);
    return shellSurface;
}

void ShellSurface::OnShellConfigure(uint32_t cx, uint32_t cy) {
    spdlog::trace("Event zwlr_layer_surface::configure {0} size {1:d}x{2:d}", m_name, cx, cy);
};

void ShellSurface::OnClosed() {
    Hide();
    // Should destroy this!
    m_isClosed = true;
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

void ShellSurface::Draw(const Anchor anchor, Buffer &buffer, const Size &size, const Rect &damage) {
    if (m_isClosed) {
        spdlog::info("Draw when closed");
        return;
    }
    if (!m_layer) Show();
    spdlog::trace("Draw buffer: {}", buffer.name);
    zwlr_layer_surface_v1_set_size(m_layer, size.cx, size.cy);
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
            break;
    }

    zwlr_layer_surface_v1_set_anchor(m_layer, zanchor);
    wl_surface_attach(m_surface, buffer.Lock(), 0, 0);
    wl_surface_damage(m_surface, damage.x, damage.y, damage.cx, damage.cy);
    wl_surface_commit(m_surface);
    // TODO: Here?
    m_roots->FlushAndDispatchCommands();
}

void ShellSurface::Hide() {
    if (!m_layer) return;
    zwlr_layer_surface_v1_destroy(m_layer);
    wl_surface_attach(m_surface, NULL, 0, 0);
    m_layer = nullptr;
    m_roots->FlushAndDispatchCommands();
}
