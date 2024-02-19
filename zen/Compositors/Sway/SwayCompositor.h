#pragma once

#include <memory>

#include "zen/MainLoop.h"
#include "zen/Sources/Sources.h"

using Visibility = std::function<void(bool visibility)>;
using AlertStates = std::map<std::string, std::set<int>>;

class SwayCompositor : public IoHandler, public Source {
   public:
    static std::shared_ptr<SwayCompositor> Connect(std::shared_ptr<MainLoop> mainloop,
                                                   Visibility visibility);
    virtual ~SwayCompositor();

    virtual bool OnRead() override;
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;

   private:
    void Initialize();
    SwayCompositor(std::shared_ptr<MainLoop> mainloop, int fd, Visibility visibility)
        : Source(), m_mainloop(mainloop), m_fd(fd), m_visibility(visibility) {}
    std::shared_ptr<MainLoop> m_mainloop;
    int m_fd;
    std::string m_payload;
    Displays m_displays;
    Visibility m_visibility;
    AlertStates m_alertStates;
};
