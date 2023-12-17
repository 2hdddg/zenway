#pragma once

#include <wayland-client-protocol.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "src/Buffer.h"
#include "src/Configuration.h"
#include "src/Sources.h"

class Output;
class Registry;
class BufferPool;

class Outputs {
   public:
    static std::unique_ptr<Outputs> Create(std::shared_ptr<Configuration> config);
    bool Initialize(const Registry& registry);
    void Add(wl_output* output);

    void Draw(const Registry& registry, const Sources& sources);
    void Hide(const Registry& registry);

    void ClickSurface(wl_surface* surface, int x, int y);

   private:
    Outputs(std::shared_ptr<Configuration> config) : m_config(config) {}
    std::map<std::string, std::shared_ptr<Output>> m_map;
    const std::shared_ptr<Configuration> m_config;
    std::unique_ptr<BufferPool> m_bufferPool;
};
