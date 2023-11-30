#pragma once

#include <spdlog/logger.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1.h>

#include <memory>

#include "Buffer.h"
#include "Configuration.h"
#include "Panel.h"
#include "Roots.h"

class ShellSurface {
   public:
    static std::unique_ptr<ShellSurface> Create(const std::shared_ptr<Roots> roots,
                                                wl_output *output,
                                                PanelConfig panelConfiguration);
    void Draw(BufferPool &bufferPool, const std::string &outputName);
    void Hide();

    void OnShellConfigure(uint32_t cx, uint32_t cy);
    void OnClosed();

    bool ClickSurface(wl_surface *surface, int x, int y);

   private:
    ShellSurface(const std::shared_ptr<Roots> roots, wl_output *output, wl_surface *surface,
                 PanelConfig panelConfiguration)
        : m_roots(roots),
          m_output(output),
          m_surface(surface),
          m_layer(nullptr),
          m_inputRegion(nullptr),
          m_isClosed(false),
          m_panelConfig(std::move(panelConfiguration)) {}
    void Show();

    const std::shared_ptr<Roots> m_roots;
    wl_output *m_output;
    wl_surface *m_surface;
    zwlr_layer_surface_v1 *m_layer;
    wl_region *m_inputRegion;
    bool m_isClosed;
    Rect m_previousDamage;
    PanelConfig m_panelConfig;
    DrawnPanel m_drawn;
};
