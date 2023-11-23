#pragma once

#include <spdlog/logger.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1.h>

#include <memory>

#include "Buffer.h"
#include "Roots.h"

enum class Anchor { Left, Right, Top, Bottom };

struct Size {
    int cx;
    int cy;
};

struct Rect {
    int x;
    int y;
    int cx;
    int cy;
};

class ShellSurface {
   public:
    static std::unique_ptr<ShellSurface> Create(const std::shared_ptr<Roots> roots,
                                                wl_output *output);
    void Draw(const Anchor anchor, Buffer &buffer, const Size &size);
    void Hide();

    void OnShellConfigure(uint32_t cx, uint32_t cy);
    void OnClosed();

   private:
    ShellSurface(const std::shared_ptr<Roots> roots, wl_output *output, wl_surface *surface)
        : m_roots(roots),
          m_output(output),
          m_surface(surface),
          m_layer(nullptr),
          m_inputRegion(nullptr),
          m_isClosed(false) {}
    void Show();

    const std::shared_ptr<Roots> m_roots;
    wl_output *m_output;
    wl_surface *m_surface;
    zwlr_layer_surface_v1 *m_layer;
    wl_region *m_inputRegion;
    bool m_isClosed;
    Rect m_previousDamage;
};
