#pragma once

#include "zen/MainLoop.h"
#include "zen/Output.h"
#include "zen/ScriptContext.h"
#include "zen/Sources/Sources.h"

class Manager : public NotificationHandler {
   public:
    static std::shared_ptr<Manager> Create(std::shared_ptr<Registry> registry);
    void SetSources(std::unique_ptr<Sources> sources) { m_sources = std::move(sources); }
    virtual ~Manager() {}
    // When a batch of IO events has been processed and sources needs to be published and/or needs
    // to redrawn
    void OnChanged() override;

    void OnAlerted() override;

    // Compositors tells manager when the overlays should be visible
    void Show();
    void Hide();

    void ClickSurface(wl_surface* surface, int x, int y);
    void WheelSurface(wl_surface* surface, int x, int y, int value);

   private:
    Manager(std::shared_ptr<Registry> registry)
        : m_registry(registry), m_isVisible(false), m_visibilityChanged(false), m_alerted(false) {}

    std::shared_ptr<Registry> m_registry;
    bool m_isVisible;
    bool m_visibilityChanged;
    std::unique_ptr<Sources> m_sources;
    bool m_alerted;
};
