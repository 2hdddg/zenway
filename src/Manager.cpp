#include "Manager.h"

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

void Manager::OnBatchProcessed() {
    // No need to redraw when not visible
    if (!m_isVisible) return;
    m_outputs->Draw(*m_sources);
    m_sources->CleanAll();
}

void Manager::Hide() {
    m_outputs->Hide();
    m_isVisible = false;
}

void Manager::Show() {
    m_isVisible = true;
    m_sources->DirtyAll();
    m_outputs->Draw(*m_sources);
}
