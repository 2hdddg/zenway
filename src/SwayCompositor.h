#pragma once

#include <memory>

#include "MainLoop.h"
#include "src/Sources.h"

using Visibility = std::function<void(bool visibility)>;

class SwayCompositor : public IoHandler, public Source {
   public:
    static std::shared_ptr<SwayCompositor> Connect(MainLoop& mainLoop, Visibility visibility);
    virtual ~SwayCompositor();

    virtual bool OnRead() override;
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;

   private:
    void Initialize();
    SwayCompositor(int fd, Visibility visibility) : m_fd(fd), m_visibility(visibility) {}
    int m_fd;
    std::string m_payload;
    Displays m_displays;
    Visibility m_visibility;
};
