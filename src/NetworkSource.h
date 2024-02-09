#pragma once

#include <memory>

#include "src/MainLoop.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

class NetworkSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<NetworkSource> Create(std::shared_ptr<MainLoop> mainloop);
    void Initialize();
    virtual bool OnRead() override;
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;
    virtual ~NetworkSource() { close(m_timerfd); }

   private:
    NetworkSource(std::shared_ptr<MainLoop> mainloop, int socket, int timerfd)
        : Source(), m_mainloop(mainloop), m_socket(socket), m_timerfd(timerfd) {}
    void ReadState();

    std::shared_ptr<MainLoop> m_mainloop;
    int m_socket;
    int m_timerfd;
    std::shared_ptr<ScriptContext> m_scriptContext;
    Networks m_networks;
};
