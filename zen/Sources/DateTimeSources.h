#pragma once

#include "zen/MainLoop.h"
#include "zen/Sources/Sources.h"

class DateSource : public Source {
   public:
    static std::shared_ptr<DateSource> Create();
    void Evaluate();
    virtual ~DateSource() {}
    void Publish(const std::string_view, ScriptContext&) override {}

   private:
    DateSource() : Source() {}
};

class TimeSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<TimeSource> Create(MainLoop& mainLoop,
                                              std::shared_ptr<DateSource> dateSource);
    virtual ~TimeSource();
    virtual bool OnRead() override;
    void Publish(const std::string_view, ScriptContext&) override {}

   private:
    TimeSource(int fd, std::shared_ptr<DateSource> dateSource)
        : Source(), m_fd(fd), m_dateSource(dateSource) {}
    int m_fd;
    std::shared_ptr<DateSource> m_dateSource;
};
