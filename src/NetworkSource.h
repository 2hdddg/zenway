#pragma once

#include <memory>

#include "src/MainLoop.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

class NetworkSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<NetworkSource> Create(MainLoop& mainLoop,
                                                 std::shared_ptr<ScriptContext> scriptContext);
    void Initialize();
    void ReadState();
    virtual bool OnRead() override;

   private:
    NetworkSource(int socket, int timerfd, std::shared_ptr<ScriptContext> scriptContext)
        : m_socket(socket), m_timerfd(timerfd), m_scriptContext(scriptContext) {}

    int m_socket;
    int m_timerfd;
    std::shared_ptr<ScriptContext> m_scriptContext;
    Networks m_networks;
};
