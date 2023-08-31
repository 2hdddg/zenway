#pragma once

#include <spdlog/logger.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1.h>

#include <memory>

#include "Buffer.h"
#include "Roots.h"

enum class Anchor { Left, Right, Top, Bottom };

class ShellSurface {
   public:
    static std::unique_ptr<ShellSurface> Create(const std::shared_ptr<Roots> roots,
                                                wl_output *output, const std::string &name);
    void Draw(const Anchor anchor, Buffer &buffer, int x, int y, int cx, int cy);
    void Hide();

    void OnShellConfigure(uint32_t cx, uint32_t cy);
    void OnClosed();

   private:
    ShellSurface(const std::shared_ptr<Roots> roots, wl_output *output, wl_surface *surface,
                 const std::string &name)
        : m_roots(roots),
          m_output(output),
          m_surface(surface),
          m_layer(nullptr),
          m_name(name),
          m_isClosed(false) {}
    void Show();

    const std::shared_ptr<Roots> m_roots;
    wl_output *m_output;
    wl_surface *m_surface;
    zwlr_layer_surface_v1 *m_layer;
    std::string m_name;
    bool m_isClosed;
};
