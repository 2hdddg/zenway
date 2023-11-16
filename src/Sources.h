#pragma once

#include <map>
#include <memory>

#include "src/ScriptContext.h"
#include "src/Source.h"

class Sources {
   public:
    static std::unique_ptr<Sources> Create(std::shared_ptr<ScriptContext> scriptContext);
    void Register(std::string_view name, std::shared_ptr<Source> source);
    bool IsRegistered(const std::string& name) { return m_sources.find(name) != m_sources.end(); }
    bool IsDirty(const std::string& name) const;
    void ClearAll() const;

   private:
    Sources(std::shared_ptr<ScriptContext> scriptContext) : m_scriptContext(scriptContext) {}
    std::shared_ptr<ScriptContext> m_scriptContext;
    std::map<std::string, std::shared_ptr<Source>> m_sources;
};
