#include "Sources.h"

#include "spdlog/spdlog.h"

std::unique_ptr<Sources> Sources::Create() { return std::unique_ptr<Sources>(new Sources()); }

void Sources::Register(std::string_view name, std::shared_ptr<Source> source) {
    m_sources[std::string(name)] = source;
}

bool Sources::IsDirty(const std::set<std::string> sources) const {
    for (const auto& name : sources) {
        if (m_sources.contains(name) && m_sources.at(name)->IsSourceDirty()) {
            spdlog::trace("Source {} is dirty", name);
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
