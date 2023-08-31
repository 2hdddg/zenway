#include "Sources.h"

std::unique_ptr<Sources> Sources::Create(std::shared_ptr<ScriptContext> scriptContext) {
    return std::unique_ptr<Sources>(new Sources(scriptContext));
}

void Sources::Register(std::string_view name, std::shared_ptr<Source> source) {
    m_scriptContext->RegisterSource(name);
    m_sources[name] = source;
}

bool Sources::IsDirty(std::string_view name) const {
    if (m_sources.contains(name)) {
        return m_sources.at(name)->IsSourceDirty();
    }
    // Unknown source..
    return false;
}
