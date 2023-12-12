#pragma once

#include <filesystem>

#include "src/MainLoop.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

class PowerSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<PowerSource> Create(std::string_view name, MainLoop& mainLoop,
                                               std::shared_ptr<ScriptContext> scriptContext);
    bool Initialize();
    void ReadState();
    virtual bool OnRead() override;
    virtual ~PowerSource();

   private:
    PowerSource(std::string_view name, int fd, std::shared_ptr<ScriptContext> scriptContext)
        : m_name(name), m_timerfd(fd), m_scriptContext(scriptContext) {}
    const std::string m_name;
    std::filesystem::path m_batteryCapacity;
    std::filesystem::path m_batteryStatus;
    std::filesystem::path m_ac;
    int m_timerfd;
    std::shared_ptr<ScriptContext> m_scriptContext;
    PowerState m_sourceState;
};
