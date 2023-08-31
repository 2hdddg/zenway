#include "PowerSource.h"

#include <spdlog/spdlog.h>
#include <sys/timerfd.h>

#include <filesystem>
#include <fstream>

std::shared_ptr<PowerSource> PowerSource::Create(MainLoop& mainLoop,
                                                 std::shared_ptr<ScriptContext> scriptContext) {
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd == -1) {
        spdlog::error("Failed to create timer: {}", strerror(errno));
        return nullptr;
    }
    itimerspec timer = {.it_interval = {.tv_sec = 60}, .it_value = {.tv_sec = 0}};
    auto ret = timerfd_settime(fd, 0, &timer, nullptr);
    if (ret == -1) {
        spdlog::error("Failed to set timer: {}", strerror(errno));
        return nullptr;
    }
    auto source = std::shared_ptr<PowerSource>(new PowerSource(fd, scriptContext));
    mainLoop.Register(fd, "PowerSource", source);
    return source;
}

bool PowerSource::Initialize() {
    // TODO: Probe that these exists
    // m_ac = "/sys/class/power_supply/AC";
    m_battery = "/sys/class/power_supply/BAT0";
    ReadState();
    return true;
}

void PowerSource::ReadState() {
    // Read current charge
    int capacity;
    {
        std::ifstream f(m_battery / "capacity");
        f >> capacity;
        spdlog::info("capacity: {}", capacity);
    }
    std::string status;
    // Read status
    {
        std::ifstream f(m_battery / "status");
        f >> status;
        spdlog::info("status: {}", status);
    }
    auto state = PowerState{
        .IsCharging = status == "Charging",
        .Capacity = (uint8_t)capacity,
    };
    m_sourceDirtyFlag = state != m_sourceState;
    if (m_sourceDirtyFlag) {
        m_sourceState = state;
        m_sourceState.Publish(*m_scriptContext);
    }
}

PowerSource::~PowerSource() { close(m_timerfd); }

void PowerSource::OnRead() {
    uint64_t ignore;
    read(m_timerfd, &ignore, sizeof(ignore));
    m_sourceDirtyFlag = true;
}
