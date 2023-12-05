#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>

#include "src/ScriptContext.h"

class Source {
   public:
    virtual bool IsSourceDirty() const { return m_sourceDirtyFlag; }
    virtual void CleanDirtySource() { m_sourceDirtyFlag = false; }
    virtual void ForceDirtySource() { m_sourceDirtyFlag = true; }

   protected:
    bool m_sourceDirtyFlag;
};

// Maintains set of sources
class Sources {
   public:
    static std::unique_ptr<Sources> Create(std::shared_ptr<ScriptContext> scriptContext);
    void Register(std::string_view name, std::shared_ptr<Source> source);
    bool IsRegistered(const std::string& name) { return m_sources.find(name) != m_sources.end(); }
    void CleanAll();
    void DirtyAll();
    bool IsDirty(const std::set<std::string> sources) const;

   private:
    Sources(std::shared_ptr<ScriptContext> scriptContext) : m_scriptContext(scriptContext) {}
    std::shared_ptr<ScriptContext> m_scriptContext;
    std::map<std::string, std::shared_ptr<Source>> m_sources;
};
