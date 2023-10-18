#pragma once

#include <string>

class Source {
   public:
    virtual bool IsSourceDirty() const { return m_sourceDirtyFlag; }
    virtual void ClearDirtySource() { m_sourceDirtyFlag = false; }

   protected:
   protected:
    bool m_sourceDirtyFlag;
};
