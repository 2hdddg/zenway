#pragma once

#include <string>

class Source {
   public:
    virtual bool IsSourceDirty() const { return m_sourceDirtyFlag; }
    virtual void CleanDirtySource() { m_sourceDirtyFlag = false; }
    virtual void ForceDirtySource() { m_sourceDirtyFlag = true; }

   protected:
    bool m_sourceDirtyFlag;
};
