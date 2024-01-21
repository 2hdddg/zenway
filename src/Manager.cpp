#include "Manager.h"

#include "spdlog/spdlog.h"
#include "src/Registry.h"

std::shared_ptr<Manager> Manager::Create(std::shared_ptr<Registry> registry) {
    auto manager = std::shared_ptr<Manager>(new Manager(registry));
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
    m_registry->BorrowOutputs().ClickSurface(surface, x, y);
}

void Manager::OnBatchProcessed() {
    // Always publish sources
    m_sources->PublishAll();
    // No need to redraw when not visible and not in transition
    if (!m_visibilityChanged && !m_isVisible) return;
    spdlog::debug("Processing batch of dirty sources");
    if (m_visibilityChanged) {
        m_visibilityChanged = false;
        if (m_isVisible) {
            m_sources->ForceRedraw();
        } else {
            m_registry->BorrowOutputs().Hide(*m_registry);
            return;
        }
    }
    m_registry->BorrowOutputs().Draw(*m_registry, *m_sources);
    m_sources->SetAllDrawn();
}

void Manager::Hide() {
    m_isVisible = false;
    m_visibilityChanged = true;
    OnBatchProcessed();
}

void Manager::Show() {
    m_isVisible = true;
    m_visibilityChanged = true;
    OnBatchProcessed();
}
