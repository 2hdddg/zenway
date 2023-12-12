#pragma once

#include <memory>

#include "MainLoop.h"
#include "src/Manager.h"

class SwayCompositor : public IoHandler {
   public:
    static std::shared_ptr<SwayCompositor> Connect(MainLoop& mainLoop,
                                                   std::shared_ptr<Manager> manager);
    virtual ~SwayCompositor();

    virtual bool OnRead() override;

   private:
    void Initialize();
    SwayCompositor(int fd, std::shared_ptr<Manager> manager) : m_fd(fd), m_manager(manager) {}
    int m_fd;
    std::shared_ptr<Manager> m_manager;
    std::string m_payload;
};
