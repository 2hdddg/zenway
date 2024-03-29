#include "zen/Sources/DateTimeSources.h"

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
    // TODO: To save some redraws the redraw flag should check if date has changed since last draw
    m_published = true;  // No need to publish
    m_drawn = false;
}

std::shared_ptr<TimeSource> TimeSource::Create(MainLoop& mainLoop,
                                               std::shared_ptr<DateSource> dateSource) {
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (fd == -1) {
        spdlog::error("Failed to create timer: {}", strerror(errno));
        return nullptr;
    }
    // Set initial timer
    auto now = Now();
    auto initial = 60 - now.tm_sec;
    itimerspec timer = {.it_interval = {.tv_sec = 60, .tv_nsec = 0},
                        .it_value = {.tv_sec = initial, .tv_nsec = 0}};
    auto ret = timerfd_settime(fd, 0, &timer, nullptr);
    if (ret == -1) {
        spdlog::error("Failed to set timer: {}", strerror(errno));
        return nullptr;
    }

    auto source = std::shared_ptr<TimeSource>(new TimeSource(fd, dateSource));
    mainLoop.RegisterIoHandler(fd, "TimeSource", source);
    return source;
}

TimeSource::~TimeSource() { close(m_fd); }

bool TimeSource::OnRead() {
    spdlog::debug("Time source set to dirty");
    uint64_t ignore;
    auto n = read(m_fd, &ignore, sizeof(ignore));
    if (n <= 0) {
        // Either block or no events
        return false;
    }
    m_published = true;  // No need to publish
    m_drawn = false;
    m_dateSource->Evaluate();
    return !m_drawn;
}
