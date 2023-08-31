#include "DateTimeSources.h"

#include <spdlog/spdlog.h>
#include <sys/timerfd.h>

using std::chrono::system_clock;

static tm Now() {
    auto now = system_clock::to_time_t(system_clock::now());
    tm local;
    localtime_r(&now, &local);
    return local;
}

std::shared_ptr<DateSource> DateSource::Create() {
    return std::shared_ptr<DateSource>(new DateSource());
}

void DateSource::Evaluate() {
    // TODO: Check dirty
}

std::shared_ptr<TimeSource> TimeSource::Create(MainLoop& mainLoop,
                                               std::shared_ptr<DateSource> dateSource) {
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd == -1) {
        spdlog::error("Failed to create timer: {}", strerror(errno));
        return nullptr;
    }
    // Set initial timer
    auto now = Now();
    auto initial = 60 - now.tm_sec;
    spdlog::debug("Setting initial timer to {}", initial);
    itimerspec timer = {.it_interval = {.tv_sec = 60}, .it_value = {.tv_sec = initial}};
    auto ret = timerfd_settime(fd, 0, &timer, nullptr);
    if (ret == -1) {
        spdlog::error("Failed to set timer: {}", strerror(errno));
        return nullptr;
    }

    auto source = std::shared_ptr<TimeSource>(new TimeSource(fd, dateSource));
    mainLoop.Register(fd, "TimeSource", source);
    return source;
}

TimeSource::~TimeSource() { close(m_fd); }

void TimeSource::OnRead() {
    spdlog::info("Timer");
    uint64_t ignore;
    read(m_fd, &ignore, sizeof(ignore));
    m_sourceDirtyFlag = true;
    m_dateSource->Evaluate();
}
