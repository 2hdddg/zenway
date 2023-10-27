#pragma once

#include "src/MainLoop.h"
#include "src/Outputs.h"
#include "src/Panel.h"

class Manager : public IoBatchHandler, public Source {
   public:
    static std::shared_ptr<Manager> Create(MainLoop& mainLoop, std::shared_ptr<Outputs> outputs,
                                           std::unique_ptr<Sources> sources,
                                           std::vector<std::unique_ptr<Panel>>&& panels);

    void Show();
    void Hide();
    void OnBatchProcessed() override;
    void DirtyWorkspace();

    bool IsSourceDirty() const override { return m_dirtyWorkspace; }
    void ClearDirtySource() override { m_dirtyWorkspace = false; }

   private:
    Manager(std::shared_ptr<Outputs> outputs, std::unique_ptr<Sources> sources,
            std::vector<std::unique_ptr<Panel>>&& panels)
        : m_outputs(outputs), m_sources(std::move(sources)), m_panels(std::move(panels)) {}

    bool m_isVisible;
    bool m_dirtyWorkspace;
    std::shared_ptr<Outputs> m_outputs;
    std::unique_ptr<Sources> m_sources;
    std::vector<std::unique_ptr<Panel>> m_panels;
};
