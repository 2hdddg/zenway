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
    m_polls.push_back(pollfd{.fd = m_eventFd, .events = POLLIN});

    do {
        // Should be ppoll
        int num = poll(m_polls.data(), m_polls.size(), -1);
        if (num < 0) {
            // Error!
            spdlog::info("Poll error in main loop");
            break;
        }
        if (num == 0) {
            // Timeout
            spdlog::info("Timeout in main loop");
            continue;
        }
        // Process
        spdlog::trace("Got {} polls", num);
        while (num--) {
            for (auto& poll : m_polls) {
                if ((poll.events & poll.revents) != 0) {
                    // Special treatment on internal events. Just empty events and
                    // rely on batch processing when all other events has been processed
                    if (poll.fd == m_eventFd) {
                        uint64_t ignore;
                        read(m_eventFd, &ignore, sizeof(uint64_t));
                        continue;
                    }
                    auto& handler = m_handlers[poll.fd];
                    // spdlog::trace("Ready to read: {}", poll.fd);
                    handler->OnRead();
                    // spdlog::trace("Done read: {}", poll.fd);
                }
            }
        }
        for (auto& handler : m_batchHandlers) {
            handler->OnBatchProcessed();
        }
    } while (m_polls.size() > 0);
    spdlog::info("Exiting main loop");
}

void MainLoop::Register(int fd, const std::string& name, std::shared_ptr<IoHandler> ioHandler) {
    m_handlers[fd] = ioHandler;
    m_polls.push_back(pollfd{.fd = fd, .events = POLLIN});
    spdlog::debug("Registering {} to {}", fd, name);
}

void MainLoop::RegisterBatchHandler(std::shared_ptr<IoBatchHandler> ioBatchHandler) {
    m_batchHandlers.push_back(ioBatchHandler);
}

void MainLoop::WakeupFromOtherThread() {
    uint64_t inc = 1;
    write(m_eventFd, &inc, sizeof(uint64_t));
}
