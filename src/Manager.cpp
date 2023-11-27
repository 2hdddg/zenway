#include "Manager.h"

#include "spdlog/spdlog.h"

std::shared_ptr<Manager> Manager::Create(MainLoop& mainLoop, std::shared_ptr<Outputs> outputs,
                                         std::unique_ptr<Sources> sources) {
    auto manager = std::shared_ptr<Manager>(new Manager(outputs, std::move(sources)));
    mainLoop.RegisterBatchHandler(manager);
    manager->m_sources->Register("workspace", manager);
    return manager;
}

void Manager::DirtyWorkspace() {
    m_sourceDirtyFlag = true;
    OnBatchProcessed();
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
            m_outputs->Hide();
            return;
        }
    }
    m_outputs->Draw(*m_sources);
    m_sources->CleanAll();
}

void Manager::Hide() {
    m_isVisible = false;
    m_visibilityChanged = true;
    // OnBatchProcessed will be invoked by main loop
    // This could be problematic on other compositors. Maybe let each compositor be the source
    // instead
}

void Manager::Show() {
    m_isVisible = true;
    m_visibilityChanged = true;
    // OnBatchProcessed will be invoked by main loop
}
