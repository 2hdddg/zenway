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
    registry->seat->RegisterHandlers(
        [manager](auto surface, int x, int y) { manager->ClickSurface(surface, x, y); },
        [manager](auto surface, int x, int y, int value) {
            manager->WheelSurface(surface, x, y, value);
        });
    return manager;
}

void Manager::ClickSurface(wl_surface* surface, int x, int y) {
    spdlog::debug("Click in surface {} at {},{}", (void*)surface, x, y);
    m_registry->BorrowOutputs().ClickSurface(surface, x, y);
}

void Manager::WheelSurface(wl_surface* surface, int x, int y, int value) {
    spdlog::debug("Wheel in surface {} at {},{},{}", (void*)surface, x, y, value);
    m_registry->BorrowOutputs().WheelSurface(surface, x, y, value);
}

void Manager::OnChanged() {
    m_sources->PublishAll();
    if (m_visibilityChanged) {
        m_visibilityChanged = false;
        if (m_isVisible) {
            m_sources->ForceRedraw();
            if (m_alerted) {
                m_registry->BorrowOutputs().HideAlert(*m_registry);
                m_alerted = false;
            }
        } else {
            m_registry->BorrowOutputs().Hide(*m_registry);
        }
    }
    if (m_isVisible) {
        m_registry->BorrowOutputs().Draw(*m_registry, *m_sources);
        m_sources->SetAllDrawn();
    } else if (m_alerted) {
        m_registry->BorrowOutputs().DrawAlert(*m_registry);
    }
}

void Manager::OnAlerted() {
    m_alerted = true;
    OnChanged();
}

void Manager::Hide() {
    m_isVisible = false;
    m_visibilityChanged = true;
    OnChanged();
}

void Manager::Show() {
    m_isVisible = true;
    m_visibilityChanged = true;
    OnChanged();
}
