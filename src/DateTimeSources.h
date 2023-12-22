#pragma once

#include "MainLoop.h"
#include "Sources.h"

class DateSource : public Source {
   public:
    static std::shared_ptr<DateSource> Create();
    void Evaluate();
    virtual ~DateSource() {}

   private:
    DateSource() {}
};

class TimeSource : public Source, public IoHandler {
   public:
    static std::shared_ptr<TimeSource> Create(MainLoop& mainLoop,
                                              std::shared_ptr<DateSource> dateSource);
    virtual ~TimeSource();
    virtual bool OnRead() override;

   private:
    TimeSource(int fd, std::shared_ptr<DateSource> dateSource)
        : m_fd(fd), m_dateSource(dateSource) {}
    int m_fd;
    std::shared_ptr<DateSource> m_dateSource;
};
