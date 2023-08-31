#pragma once

#include "Configuration.h"
#include "cairo.h"
#include "pango/pango-layout.h"
#include "src/BufferPool.h"
#include "src/Output.h"
#include "src/Sources.h"

class Panel {
   public:
    static std::unique_ptr<Panel> Create(std::shared_ptr<BufferPool> bufferPool,
                                         Configuration::Panel panelConfig,
                                         std::shared_ptr<Sources> sources);
    void Draw(Output& output);
    bool IsDirty() const;
    // void Draw(std::map<std::string, std::shared_ptr<Source>> sources);

   private:
    Panel(std::shared_ptr<BufferPool> bufferPool, Configuration::Panel panelConfig,
          std::shared_ptr<Sources> sources)
        : m_bufferPool(bufferPool), m_panelConfig(panelConfig), m_sources(sources) {}
    const std::shared_ptr<BufferPool> m_bufferPool;
    const Configuration::Panel m_panelConfig;
    const std::shared_ptr<Sources> m_sources;
};
