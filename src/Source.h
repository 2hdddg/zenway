#pragma once

#include <string>

// template <typename TState>
class Source {
   public:
    // const std::string& SourceName() { return m_sourceName; }
    virtual bool IsSourceDirty() const { return m_sourceDirtyFlag; }
    virtual void ClearDirtySource() { m_sourceDirtyFlag = false; }
    // virtual TState GetSourceState() { return m_sourceState; }

   protected:
    // Source(const char* name) : m_sourceName(name) {}

   protected:
    // std::string m_sourceName;
    //  TState m_sourceState;
    bool m_sourceDirtyFlag;
};
