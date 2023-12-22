#pragma once

#include "src/MainLoop.h"
#include "src/Output.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

class Manager : public IoBatchHandler, public Source {
   public:
    static std::shared_ptr<Manager> Create(std::shared_ptr<Registry> registry,
                                           std::string_view sourceName, MainLoop& mainLoop,
                                           std::unique_ptr<Sources> sources,
                                           std::shared_ptr<ScriptContext> scriptContext);
    virtual ~Manager() {}
    // When a batch of IO events has been processed and sources are potentially dirty
    void OnBatchProcessed() override;

    // Used by compositor implementation
    void Show();
    void Hide();
    void Publish(const Displays& displays);

    void ClickSurface(wl_surface* surface, int x, int y);

   private:
    Manager(std::shared_ptr<Registry> registry, std::string_view sourceName,
            std::unique_ptr<Sources> sources, std::shared_ptr<ScriptContext> scriptContext)
        : m_registry(registry),
          m_sourceName(sourceName),
          m_sources(std::move(sources)),
          m_scriptContext(scriptContext) {}

    std::shared_ptr<Registry> m_registry;
    std::string m_sourceName;
    bool m_isVisible;
    bool m_visibilityChanged;
    std::unique_ptr<Sources> m_sources;
    std::shared_ptr<ScriptContext> m_scriptContext;
};
