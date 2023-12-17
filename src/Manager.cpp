#include "Manager.h"

#include "spdlog/spdlog.h"
#include "src/Registry.h"

std::shared_ptr<Manager> Manager::Create(std::shared_ptr<Registry> registry,
                                         std::string_view sourceName, MainLoop& mainLoop,
                                         std::shared_ptr<Outputs> outputs,
                                         std::unique_ptr<Sources> sources,
                                         std::shared_ptr<ScriptContext> scriptContext) {
    auto manager = std::shared_ptr<Manager>(
        new Manager(registry, sourceName, outputs, std::move(sources), scriptContext));
    // Register as a source
    manager->m_sources->Register(sourceName, manager);
    // Register source batch handler
    mainLoop.RegisterBatchHandler(manager);
    // Register click handler
    if (!registry->seat) {
        spdlog::error("No seat in registry");
        return nullptr;
    }
    registry->seat->RegisterClickHandler(
        [manager](auto surface, int x, int y) { manager->ClickSurface(surface, x, y); });
    return manager;
}

void Manager::ClickSurface(wl_surface* surface, int x, int y) {
    spdlog::debug("Click in surface {} at {},{}", (void*)surface, x, y);
    m_outputs->ClickSurface(surface, x, y);
}

void Manager::OnBatchProcessed() {
    // No need to redraw when not visible and not in transition
    if (!m_visibilityChanged && !m_isVisible) return;

    spdlog::debug("Processing batch of dirty sources");
    if (m_visibilityChanged) {
        m_visibilityChanged = false;
        if (m_isVisible) {
            m_sources->DirtyAll();
        } else {
            m_outputs->Hide(*m_registry);
            return;
        }
    }
    m_outputs->Draw(*m_registry, *m_sources);
    m_sources->CleanAll();
}

void Manager::Publish(const Displays& displays) {
    m_scriptContext->Publish(m_sourceName, displays);
    m_sourceDirtyFlag = true;
    OnBatchProcessed();
}

void Manager::Hide() {
    m_isVisible = false;
    m_visibilityChanged = true;
    m_sourceDirtyFlag = true;
    OnBatchProcessed();
}

void Manager::Show() {
    m_isVisible = true;
    m_visibilityChanged = true;
    m_sourceDirtyFlag = true;
    OnBatchProcessed();
}
