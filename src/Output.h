#pragma once

#include <wayland-client-protocol.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "Buffer.h"
#include "Roots.h"
#include "ShellSurface.h"
#include "src/Configuration.h"
#include "src/Sources.h"

class Output;

class Outputs {
   public:
    static std::unique_ptr<Outputs> Create(std::shared_ptr<Configuration> config);
    bool Initialize(const std::shared_ptr<Roots> roots);
    void Add(wl_output* output);

    void Draw(const Sources& sources);
    void Hide();

    void ClickSurface(wl_surface* surface, int x, int y);

   private:
    Outputs(std::shared_ptr<Configuration> config) : m_config(config) {}
    std::map<std::string, std::shared_ptr<Output>> m_map;
    std::shared_ptr<Roots> m_roots;
    const std::shared_ptr<Configuration> m_config;
    std::unique_ptr<BufferPool> m_bufferPool;
};
