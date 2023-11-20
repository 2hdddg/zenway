#pragma once

#include "Configuration.h"
#include "cairo.h"
#include "pango/pango-layout.h"
#include "src/Buffer.h"
#include "src/Output.h"
#include "src/Sources.h"

class Panel {
   public:
    static std::unique_ptr<Panel> Create(std::shared_ptr<BufferPool> bufferPool,
                                         Configuration::Panel panelConfig);
    void Draw(Output& output);
    bool IsDirty(const Sources& sources) const;

   private:
    Panel(std::shared_ptr<BufferPool> bufferPool, Configuration::Panel panelConfig)
        : m_bufferPool(bufferPool), m_panelConfig(panelConfig), m_previousDamage(0, 0, 0, 0) {}
    const std::shared_ptr<BufferPool> m_bufferPool;
    const Configuration::Panel m_panelConfig;
    Rect m_previousDamage;
};
