#pragma once

#include <spdlog/logger.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1.h>

#include <memory>

#include "Configuration.h"
#include "Draw.h"

class Registry;

class ShellSurface {
   public:
    static std::unique_ptr<ShellSurface> Create(const Registry &registry, wl_output *output,
                                                PanelConfig panelConfiguration);
    void Draw(const Registry &registry, BufferPool &bufferPool, const std::string &outputName);
    void Hide(const Registry &registry);

    void OnShellConfigure(uint32_t cx, uint32_t cy);
    void OnClosed();

    bool ClickSurface(wl_surface *surface, int x, int y);
    bool WheelSurface(wl_surface *surface, int x, int y, int value);

   private:
    ShellSurface(wl_output *output, wl_surface *surface, PanelConfig panelConfiguration)
        : m_output(output),
          m_surface(surface),
          m_layer(nullptr),
          m_inputRegion(nullptr),
          m_isClosed(false),
          m_previousDamage{},
          m_panelConfig(std::move(panelConfiguration)) {}

    wl_output *m_output;
    wl_surface *m_surface;
    zwlr_layer_surface_v1 *m_layer;
    wl_region *m_inputRegion;
    bool m_isClosed;
    Size m_previousDamage;
    PanelConfig m_panelConfig;
    DrawnPanel m_drawn;
};
