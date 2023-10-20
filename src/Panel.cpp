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

struct Renderable {
    virtual ~Renderable() {}
    virtual void Compute(cairo_t* cr) {}
    virtual void Draw(cairo_t* cr) const {}
    Computed computed;
};

struct Markup : public Renderable {
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

struct MarkupBox : public Renderable {
    MarkupBox(const std::string& string) : markup(string) {}

   private:
    Markup markup;
    uint32_t background_color;
    Border border;
};

struct Meter : public Renderable {
    Border border;
};

std::unique_ptr<Renderable> FromObject(const sol::object& o);

void FromChildTable(const sol::table childTable,
                    std::vector<std::unique_ptr<Renderable>>& children) {
    size_t size = childTable.size();
    for (size_t i = 0; i < size; i++) {
        const sol::object& o = childTable[i + 1];
        auto b = FromObject(o);
        if (b) {
            children.push_back(std::move(b));
        }
    }
}

struct FlexContainer : public Renderable {
    static std::unique_ptr<Renderable> FromTable(const sol::table& t) {
        FlexContainer f;
        const sol::optional<std::string> direction = t["direction"];
        f.isColumn = direction ? *direction == "column" : true;
        sol::optional<sol::table> children = t["items"];
        if (children) {
            FromChildTable(*children, f.children);
        }
        return std::unique_ptr<Renderable>(new FlexContainer(std::move(f)));
    }
    void Compute(cairo_t* cr) override {
        computed.cx = 0;
        computed.cy = 0;
        for (const auto& r : children) {
            r->Compute(cr);
            computed.cx += r->computed.cx;
            computed.cy += r->computed.cy;
        }
    }
    void Draw(cairo_t* cr) const override {
        // x
        //
        int x = 0;
        int y = 0;
        for (const auto& r : children) {
            r->Draw(cr);
            y += r->computed.cy;
            cairo_move_to(cr, x, y);
        }
    }

   private:
    bool isColumn;
    std::vector<std::unique_ptr<Renderable>> children;
};

std::unique_ptr<Renderable> FromObject(const sol::object& o) {
    if (o.is<std::string>()) {
        return std::make_unique<Markup>(o.as<std::string>());
    }
    if (!o.is<sol::table>()) {
        return nullptr;
    }
    const auto& t = o.as<sol::table>();
    const sol::optional<std::string> type = t["type"];
    if (!type) {
        return nullptr;
    }
    if (*type == "flex") {
        return FlexContainer::FromTable(t);
    }
    return nullptr;
}

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
        sol::object renderOutput = widgetConfig.render(output.name);
        auto item = FromObject(renderOutput);
        if (!item) {
            continue;
        }
        item->Compute(cr);
        auto widgetCx = item->computed.cx;
        if (alignRight) {
            cairo_move_to(cr, bufferCx - widgetCx - m_panelConfig.screenBorderOffset, y);
        } else {
            cairo_move_to(cr, m_panelConfig.screenBorderOffset, y);
        }
        item->Draw(cr);
        cairo_restore(cr);
        y += item->computed.cy;
        y += 10;
    }
    output.surfaces[m_panelConfig.index]->Draw(m_panelConfig.anchor, *buffer, 0, 0, bufferCx, y);
}
