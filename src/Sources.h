#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>

#include "src/ScriptContext.h"

class Source {
   public:
    void SetDrawn() { m_drawn = true; }
    void ClearDrawn() { m_drawn = false; }
    bool IsDrawn() { return m_drawn; }

    virtual void Publish(const std::string_view sourceName, ScriptContext& scriptContext) = 0;
    virtual ~Source() {}

   protected:
    bool m_drawn;      // Set to false to indicate that source needs to be redrawn
    bool m_published;  // Set to false to indicate that there is a new state to be published
};

// Maintains set of sources
class Sources {
   public:
    static std::unique_ptr<Sources> Create(std::unique_ptr<ScriptContext> scriptContext);
    void Register(std::string_view name, std::shared_ptr<Source> source);
    bool IsRegistered(const std::string& name) { return m_sources.find(name) != m_sources.end(); }
    void SetAllDrawn();
    void ForceRedraw();
    bool NeedsRedraw(const std::set<std::string> sources) const;
    void PublishAll();

   private:
    Sources(std::unique_ptr<ScriptContext> scriptContext)
        : m_scriptContext(std::move(scriptContext)) {}
    std::map<std::string, std::shared_ptr<Source>> m_sources;
    std::unique_ptr<ScriptContext> m_scriptContext;
};
