#include "MainLoop.h"

#include <spdlog/spdlog.h>
#include <sys/eventfd.h>

std::unique_ptr<MainLoop> MainLoop::Create() {
    // Open event channel
    int eventFd = eventfd(0, EFD_NONBLOCK);
    if (eventFd == -1) {
        return nullptr;
    }
    return std::unique_ptr<MainLoop>(new MainLoop(eventFd));
}

void MainLoop::Run() {
    // Register internal events
    m_polls.push_back(pollfd{.fd = m_wakeupFd, .events = POLLIN, .revents = 0});

    do {
        // Should be ppoll
        int num = poll(m_polls.data(), m_polls.size(), -1);
        if (num < 0) {
            // Error!
            spdlog::error("Poll error in main loop");
            break;
        }
        if (num == 0) {
            // Timeout
            spdlog::error("Timeout in main loop");
            continue;
        }
        // Process
        spdlog::trace("{} events in main loop", num);
        bool anyDirty = false;
        for (auto& poll : m_polls) {
            if ((poll.events & poll.revents) != 0) {
                // Special treatment on internal events. Empty events and
                // rely on batch processing when all other events has been processed
                if (poll.fd == m_wakeupFd) {
                    uint64_t ignore;
                    spdlog::trace("Popping internal event");
                    m_wakupMutex.lock();
                    read(m_wakeupFd, &ignore, sizeof(uint64_t));
                    m_wakupMutex.unlock();
                    // For now internal events always means that a source is dirty
                    anyDirty = true;
                } else {
                    auto& handler = m_handlers[poll.fd];
                    spdlog::trace("Invoking io handler for fd {}", poll.fd);
                    anyDirty = handler->OnRead() || anyDirty;
                    spdlog::trace("Io handler done");
                }
            }
        }
        if (anyDirty && m_handler) {
            m_handler->OnChanged();
        }
        if (m_alerted) {
            m_handler->OnAlerted();
            m_alerted = false;
        }
    } while (m_polls.size() > 0);
}

void MainLoop::RegisterIoHandler(int fd, const std::string_view name,
                                 std::shared_ptr<IoHandler> ioHandler) {
    m_handlers[fd] = ioHandler;
    m_polls.push_back(pollfd{.fd = fd, .events = POLLIN, .revents = 0});
    spdlog::debug("Registering {} in main loop for fd {}", name, fd);
}

void MainLoop::RegisterNotificationHandler(std::shared_ptr<NotificationHandler> batchHandler) {
    if (m_handler) {
        spdlog::error("Only one batch handler supported");
    }
    m_handler = batchHandler;
}

void MainLoop::Wakeup() {
    m_wakupMutex.lock();
    uint64_t inc = 1;
    write(m_wakeupFd, &inc, sizeof(uint64_t));
    m_wakupMutex.unlock();
}

void MainLoop::AlertAndWakeup() {
    m_alerted = true;
    Wakeup();
}
