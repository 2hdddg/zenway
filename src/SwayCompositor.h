#pragma once

#include <memory>

#include "Compositor.h"
#include "MainLoop.h"
#include "src/Manager.h"

class SwayCompositor : public Compositor, public IoHandler {
   public:
    static std::shared_ptr<SwayCompositor> Connect(MainLoop& mainLoop,
                                                   std::shared_ptr<Manager> manager,
                                                   std::shared_ptr<ScriptContext> scriptContext);
    virtual ~SwayCompositor();

    virtual void OnRead() override;

   private:
    void Initialize();
    SwayCompositor(int fd, std::shared_ptr<Manager> manager,
                   std::shared_ptr<ScriptContext> scriptContext)
        : m_fd(fd), m_manager(manager), m_scriptContext(scriptContext) {}
    int m_fd;
    std::shared_ptr<Manager> m_manager;
    std::shared_ptr<ScriptContext> m_scriptContext;
    std::string m_payload;
};
