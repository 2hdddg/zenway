#pragma once

#include <filesystem>

#include "zen/MainLoop.h"
#include "zen/ScriptContext.h"
#include "zen/Sources/Sources.h"

class PowerSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<PowerSource> Create(std::shared_ptr<MainLoop> mainloop);
    bool Initialize();
    void ReadState();
    virtual bool OnRead() override;
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;
    virtual ~PowerSource();

   private:
    PowerSource(std::shared_ptr<MainLoop> mainloop, int fd)
        : Source(), m_mainloop(mainloop), m_timerfd(fd) {}
    std::shared_ptr<MainLoop> m_mainloop;
    std::filesystem::path m_batteryCapacity;
    std::filesystem::path m_batteryStatus;
    std::filesystem::path m_ac;
    int m_timerfd;
    PowerState m_sourceState;
};
