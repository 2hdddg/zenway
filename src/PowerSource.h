#pragma once

#include <filesystem>

#include "src/MainLoop.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

class PowerSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<PowerSource> Create(MainLoop& mainLoop);
    bool Initialize();
    void ReadState();
    virtual bool OnRead() override;
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;
    virtual ~PowerSource();

   private:
    PowerSource(int fd) : m_timerfd(fd) {}
    std::filesystem::path m_batteryCapacity;
    std::filesystem::path m_batteryStatus;
    std::filesystem::path m_ac;
    int m_timerfd;
    PowerState m_sourceState;
};
