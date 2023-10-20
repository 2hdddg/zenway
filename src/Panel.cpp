#include "Panel.h"

#include "pango/pango-layout.h"
#include "pango/pangocairo.h"
#include "src/BufferPool.h"

struct Computed {
    uint32_t cx;
    uint32_t cy;
};

struct Current {
    uint32_t cx;
    uint32_t cy;
    uint32_t x;
    uint32_t y;
};

struct Border {
    uint32_t color;
    uint8_t width;
    uint8_t radius;
};

struct Padding {
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
};

struct Data {
    Computed computed;
    Current current;
};

struct Base {
    virtual void Compute(cairo_t* cr) {}
    virtual void Draw(cairo_t* cr) const {}
    virtual ~Base() {}
    Computed computed;
};

struct Markup : public Base {
    Markup(const std::string& string) : string(string) {}
    virtual ~Markup() { g_object_unref(m_layout); }
    void Compute(cairo_t* cr) override {
        // pango_layout_set_width(m_layout, m_config.cx * PANGO_SCALE);
        // pango_layout_set_height(m_layout, m_config.cy * PANGO_SCALE);
        m_layout = pango_cairo_create_layout(cr);
        pango_layout_set_markup(m_layout, string.c_str(), -1);
        PangoRectangle rect;
        pango_layout_get_extents(m_layout, nullptr, &rect);
        pango_extents_to_pixels(nullptr, &rect);
        computed.cx = rect.width;
        computed.cy = rect.height;
    }
    const std::string string;

   private:
    void Draw(cairo_t* cr) const override { pango_cairo_show_layout(cr, m_layout); }
    PangoLayout* m_layout;
};

struct MarkupBox : public Base {
    MarkupBox(const std::string& string) : markup(string) {}

   private:
    Markup markup;
    uint32_t background_color;
    Border border;
};

struct Meter : public Base {
    Border border;
};

struct FlexContainer : public Base {
    bool isColumn;
    std::vector<std::unique_ptr<Base>> children;
};

class Widget {
   public:
    Widget(const std::string& markup) : m_item(new Markup(markup), {}) {}
    void Compute(cairo_t* cr) { m_item->Compute(cr); }
    Computed GetComputed() { return m_item->computed; }
    void Draw(cairo_t* cr) const { m_item->Draw(cr); }

    virtual ~Widget() {}

   private:
    std::unique_ptr<Base> m_item;
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
        widgetCx = widget.GetComputed().cx;
        if (alignRight) {
            cairo_move_to(cr, bufferCx - widgetCx - m_panelConfig.screenBorderOffset, y);
        } else {
            cairo_move_to(cr, m_panelConfig.screenBorderOffset, y);
        }
        widget.Draw(cr);
        cairo_restore(cr);
        y += widget.GetComputed().cy;
        y += 10;
    }
    output.surfaces[m_panelConfig.index]->Draw(m_panelConfig.anchor, *buffer, 0, 0, bufferCx, y);
}
