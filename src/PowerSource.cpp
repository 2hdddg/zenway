#include "PowerSource.h"

#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/timerfd.h>

#include <filesystem>
#include <string>

std::shared_ptr<PowerSource> PowerSource::Create(MainLoop& mainLoop,
                                                 std::shared_ptr<ScriptContext> scriptContext) {
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd == -1) {
        spdlog::error("Failed to create timer: {}", strerror(errno));
        return nullptr;
    }
    itimerspec timer = {.it_interval = {.tv_sec = 30}, .it_value = {.tv_sec = 1}};
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
    m_ac = "/sys/class/power_supply/AC/online";
    // TODO: Always BAT0?
    // TODO: More than one battery?
    m_batteryCapacity = "/sys/class/power_supply/BAT0/capacity";
    m_batteryStatus = "/sys/class/power_supply/BAT0/status";
    ReadState();
    return true;
}

static std::optional<std::string> ReadAll(int fd) {
    constexpr size_t size = 512;
    char buf[size];
    auto n = read(fd, buf, size);
    if (n == -1) {
        // Probably blocking..
        return nullptr;
    }
    if (n == size) {
        // Read exact amount, might be more, not sure
        return nullptr;
    }
    return std::string(buf, n);
}

void PowerSource::ReadState() {
    auto state = PowerState{};
    // Use non-blocking to avoid sporadic hanging
    // Current capacity
    {
        auto fd = open(m_batteryCapacity.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            spdlog::error("Failed to check battery capacity");
            return;
        }
        auto maybeCapacityString = ReadAll(fd);
        close(fd);
        if (!maybeCapacityString) {
            spdlog::warn("Blocking read of capacity");
            return;
        }
        state.Capacity = std::stoi(*maybeCapacityString);
    }
    // Status
    {
        auto fd = open(m_batteryStatus.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            spdlog::error("Failed to check battery status");
            return;
        }
        auto maybeStatus = ReadAll(fd);
        close(fd);
        if (!maybeStatus) {
            spdlog::warn("Blocking read of status");
            return;
        }
        state.IsCharging = *maybeStatus == "Charging";
    }
    // AC status
    {
        auto fd = open(m_ac.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            spdlog::error("Failed to check AC online status");
            return;
        }
        auto maybeOnline = ReadAll(fd);
        close(fd);
        if (!maybeOnline) {
            spdlog::warn("Blocking read of AC online");
            return;
        }
        state.IsPluggedIn = std::stoi(*maybeOnline) != 0;
    }
    m_sourceDirtyFlag = state != m_sourceState;
    if (m_sourceDirtyFlag) {
        m_sourceState = state;
        m_scriptContext->Publish(m_sourceState);
        spdlog::info("Power status changed, capacity {}, charging {}, plugged in {}",
                     state.Capacity, state.IsCharging, state.IsPluggedIn);
    }
}

PowerSource::~PowerSource() { close(m_timerfd); }

bool PowerSource::OnRead() {
    spdlog::debug("Polling power status");
    uint64_t ignore;
    read(m_timerfd, &ignore, sizeof(ignore));
    ReadState();
    return m_sourceDirtyFlag;
}
