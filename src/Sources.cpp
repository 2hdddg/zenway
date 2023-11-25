#include "Sources.h"

std::unique_ptr<Sources> Sources::Create(std::shared_ptr<ScriptContext> scriptContext) {
    return std::unique_ptr<Sources>(new Sources(scriptContext));
}

void Sources::Register(std::string_view name, std::shared_ptr<Source> source) {
    m_scriptContext->RegisterSource(name);
    m_sources[std::string(name)] = source;
}

bool Sources::IsDirty(const std::set<std::string> sources) const {
    for (const auto& name : sources) {
        if (m_sources.contains(name) && m_sources.at(name)->IsSourceDirty()) {
            return true;
        }
    }
    return false;
}

void Sources::CleanAll() {
    for (auto const& source : m_sources) {
        source.second->CleanDirtySource();
    }
}

void Sources::DirtyAll() {
    for (auto const& source : m_sources) {
        source.second->ForceDirtySource();
    }
}
