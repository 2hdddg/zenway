#pragma once

#include <poll.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

class IoHandler {
   public:
    virtual ~IoHandler() {}
    // Return true if read cases handler to be dirty
    virtual bool OnRead() = 0;
};

class NotificationHandler {
   public:
    virtual ~NotificationHandler() {}
    virtual void OnChanged() = 0;
    virtual void OnAlerted() = 0;
};

class MainLoop {
   public:
    static std::unique_ptr<MainLoop> Create();

    void Run();
    void RegisterIoHandler(int fd, const std::string_view name,
                           std::shared_ptr<IoHandler> ioHandler);
    void RegisterNotificationHandler(std::shared_ptr<NotificationHandler> ioBatchHandler);
    // In cases where polling for events is done on another thread that thread should
    // call this to trigger dirty check on all registered io handlers.
    // Or in cases where source gets dirty due to some other type of event than io.
    void Wakeup();

    void AlertAndWakeup();

   private:
    MainLoop(int eventFd) : m_wakeupFd(eventFd) {}

    int m_wakeupFd;
    std::mutex m_wakupMutex;
    std::atomic<bool> m_alerted;
    std::vector<pollfd> m_polls;
    std::map<int, std::shared_ptr<IoHandler>> m_handlers;
    std::shared_ptr<NotificationHandler> m_handler;
};
