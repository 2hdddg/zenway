#include "zen/Sources/Sources.h"

#include "spdlog/spdlog.h"

std::unique_ptr<Sources> Sources::Create(std::unique_ptr<ScriptContext> scriptContext) {
    return std::unique_ptr<Sources>(new Sources(std::move(scriptContext)));
}

void Sources::Register(std::string_view name, std::shared_ptr<Source> source) {
    m_sources[std::string(name)] = source;
}

bool Sources::NeedsRedraw(const std::set<std::string> sources) const {
    for (const auto& name : sources) {
        if (m_sources.contains(name) && !m_sources.at(name)->IsDrawn()) {
            spdlog::trace("Source {} needs render", name);
            return true;
        }
    }
    return false;
}

void Sources::SetAllDrawn() {
    for (auto const& source : m_sources) {
        source.second->SetDrawn();
    }
}

void Sources::ForceRedraw() {
    for (auto const& source : m_sources) {
        source.second->ClearDrawn();
    }
}

void Sources::PublishAll() {
    for (auto const& source : m_sources) {
        source.second->Publish(source.first, *m_scriptContext);
    }
}
