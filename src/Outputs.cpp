#include "Outputs.h"

#include <spdlog/spdlog.h>

#include "Output.h"

std::unique_ptr<Outputs> Outputs::Create(const std::shared_ptr<Roots> roots) {
    return std::unique_ptr<Outputs>(new Outputs(roots));
}

void Outputs::Add(wl_output* wloutput) {
    Output::Create(m_roots, wloutput, [this](Output* output) {
        spdlog::debug("Found output {}", output->name);
        m_map[output->name] = std::shared_ptr<Output>(output);
    });
}

void Outputs::ForEach(std::function<void(std::shared_ptr<Output>)> callback) {
    for (auto& keyValue : m_map) {
        callback(keyValue.second);
    }
}
std::shared_ptr<Output> Outputs::Get(const std::string& name) const {
    auto keyValue = m_map.find(name);
    if (keyValue == m_map.end()) return nullptr;
    return keyValue->second;
}
