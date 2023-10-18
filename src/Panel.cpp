#include "Panel.h"

#include "pango/pango-layout.h"
#include "pango/pangocairo.h"
#include "src/BufferPool.h"

class Box {};

struct Computed {
    int cx;
    int cy;
};

class Widget {
   public:
    Widget(const std::string& markup) : m_markupOrBox(markup) {}
    void Compute(cairo_t* cr) {
        m_layout = pango_cairo_create_layout(cr);
        // pango_layout_set_width(m_layout, m_config.cx * PANGO_SCALE);
        // pango_layout_set_height(m_layout, m_config.cy * PANGO_SCALE);
        const std::string* markup = std::get_if<std::string>(&m_markupOrBox);
        if (!markup) {
            return;
        }
        pango_layout_set_markup(m_layout, markup->c_str(), -1);
        PangoRectangle rect;
        pango_layout_get_extents(m_layout, nullptr, &rect);
        pango_extents_to_pixels(nullptr, &rect);
        computed.cx = rect.width;
        computed.cy = rect.height;
    }
    Computed computed;
    void Draw(cairo_t* cr) const { pango_cairo_show_layout(cr, m_layout); }

    virtual ~Widget() { g_object_unref(m_layout); }

   private:
    std::variant<std::string, Box> m_markupOrBox;
    PangoLayout* m_layout;
};

std::unique_ptr<Panel> Panel::Create(std::shared_ptr<BufferPool> bufferPool,
                                     Configuration::Panel panelConfig,
                                     std::shared_ptr<Sources> sources) {
    return std::unique_ptr<Panel>(new Panel(bufferPool, panelConfig, sources));
}

bool Panel::IsDirty() const {
    for (auto& widget : m_panelConfig.widgets) {
        for (const auto& sourceName : widget.sources) {
            if (m_sources->IsDirty(sourceName)) return true;
        }
    }
    return false;
}

void Panel::Draw(Output& output) {
    auto y = 0;
    // Get free buffer to draw in. This could fail if both buffers are locked.
    auto buffer = m_bufferPool->Get();
    if (!buffer) {
        perror("No buffer to draw in");
        return;
    }
    // Draw to buffer
    auto cr = buffer->GetCairoCtx();
    buffer->Clear(0x00);
    auto bufferCx = buffer->Cx();
    bool alignRight = m_panelConfig.anchor == Anchor::Right;

    for (auto& widgetConfig : m_panelConfig.widgets) {
        cairo_save(cr);

        // if (IsDirty(widget, *m_sources)) {
        auto widgetCx = 0;
        sol::object renderOutput = widgetConfig.render(output.name);
        if (!renderOutput.is<std::string>()) {
            continue;
        }
        Widget widget(renderOutput.as<std::string>());
        widget.Compute(cr);
        widgetCx = widget.computed.cx;
        if (alignRight) {
            cairo_move_to(cr, bufferCx - widgetCx - m_panelConfig.screenBorderOffset, y);
        } else {
            cairo_move_to(cr, m_panelConfig.screenBorderOffset, y);
        }
        widget.Draw(cr);
        cairo_restore(cr);
        y += widget.computed.cy;
        y += 10;
    }
    output.surfaces[m_panelConfig.index]->Draw(m_panelConfig.anchor, *buffer, 0, 0, bufferCx, y);
}
