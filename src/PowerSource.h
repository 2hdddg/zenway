#pragma once

#include <filesystem>

#include "src/MainLoop.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

class PowerSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<PowerSource> Create(MainLoop& mainLoop,
                                               std::shared_ptr<ScriptContext> scriptContext);
    bool Initialize();
    void ReadState();
    virtual bool OnRead() override;
    virtual ~PowerSource();

   private:
    PowerSource(int fd, std::shared_ptr<ScriptContext> scriptContext)
        : m_timerfd(fd), m_scriptContext(scriptContext) {}
    std::filesystem::path m_battery;
    std::filesystem::path m_ac;
    int m_timerfd;
    std::shared_ptr<ScriptContext> m_scriptContext;
    PowerState m_sourceState;
};
