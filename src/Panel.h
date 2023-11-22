#pragma once

#include "Configuration.h"
#include "cairo.h"
#include "pango/pango-layout.h"
#include "src/Buffer.h"
#include "src/Output.h"
#include "src/Sources.h"

struct Renderable {
    virtual ~Renderable() {}
    virtual void Compute(cairo_t* cr) {}
    virtual void Draw(cairo_t* cr, int x, int y) const {}
    Size computed;
};

struct Widget {
    void Compute(const Configuration::Widget& config, const Output& output, cairo_t* cr);
    void Draw(cairo_t* cr, int x, int y) const;
    Size computed;

   private:
    std::unique_ptr<Renderable> m_item;
    int m_paddingX;
    int m_paddingY;
};

class Panel {
   public:
    static std::unique_ptr<Panel> Create(std::shared_ptr<BufferPool> bufferPool,
                                         Configuration::Panel panelConfig);
    void Draw(Output& output);
    bool IsDirty(const Sources& sources) const;

   private:
    Panel(std::shared_ptr<BufferPool> bufferPool, Configuration::Panel panelConfig)
        : m_bufferPool(bufferPool),
          m_panelConfig(panelConfig),
          m_widgets(panelConfig.widgets.size()) {}
    const std::shared_ptr<BufferPool> m_bufferPool;
    const Configuration::Panel m_panelConfig;
    std::vector<Widget> m_widgets;
};
