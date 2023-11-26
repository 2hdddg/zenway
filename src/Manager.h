#pragma once

#include "src/MainLoop.h"
#include "src/Output.h"
#include "src/Panel.h"

class Manager : public IoBatchHandler, public Source {
   public:
    static std::shared_ptr<Manager> Create(MainLoop& mainLoop, std::shared_ptr<Outputs> outputs,
                                           std::unique_ptr<Sources> sources);
    // When a batch of IO events has been processed and sources are potentially dirty
    void OnBatchProcessed() override;

    // Used by compositor implementation
    void Show();
    void Hide();
    void DirtyWorkspace();

   private:
    Manager(std::shared_ptr<Outputs> outputs, std::unique_ptr<Sources> sources)
        : m_outputs(outputs), m_sources(std::move(sources)) {}

    bool m_isVisible;
    bool m_visibilityChanged;
    std::shared_ptr<Outputs> m_outputs;
    std::unique_ptr<Sources> m_sources;
};
