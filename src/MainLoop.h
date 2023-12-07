#pragma once

#include <poll.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

class IoHandler {
   public:
    // Return true if read cases handler to be dirty
    virtual bool OnRead() = 0;
};

class IoBatchHandler {
   public:
    virtual void OnBatchProcessed() = 0;
};

class MainLoop {
   public:
    static std::unique_ptr<MainLoop> Create();

    void Run();
    void Register(int fd, const std::string& name, std::shared_ptr<IoHandler> ioHandler);
    void RegisterBatchHandler(std::shared_ptr<IoBatchHandler> ioBatchHandler);
    // In cases where polling for events is done on another thread that thread should
    // call this to trigger dirty check on all registered batch handlers.
    // Or in cases where source gets dirty due to some other type of event
    void Wakeup();

   private:
    MainLoop(int eventFd) : m_eventFd(eventFd) {}

    int m_eventFd;
    std::vector<pollfd> m_polls;
    std::map<int, std::shared_ptr<IoHandler>> m_handlers;
    std::vector<std::shared_ptr<IoBatchHandler>> m_batchHandlers;
};
