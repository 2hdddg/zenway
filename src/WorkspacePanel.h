#pragma once

#include <memory>

#include "BufferPool.h"
#include "MainLoop.h"
#include "Registry.h"
#include "Theme.h"
#include "Workspaces.h"

class WorkspacePanel {
   public:
    static std::unique_ptr<WorkspacePanel> Create(std::shared_ptr<BufferPool> bufferPool,
                                                  MainLoop& mainLoop,
                                                  const std::shared_ptr<Theme> theme);
    void Draw(Output& output);

   private:
    WorkspacePanel(std::shared_ptr<BufferPool> bufferPool, const std::shared_ptr<Theme> theme)
        : m_bufferPool(std::move(bufferPool)), m_theme(theme) {}

    const std::shared_ptr<BufferPool> m_bufferPool;
    const std::shared_ptr<Theme> m_theme;
};
