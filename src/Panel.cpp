#include "Panel.h"

#include "pango/pango-layout.h"
#include "pango/pangocairo.h"
#include "spdlog/spdlog.h"

struct RGBA {
    static RGBA FromProperty(const sol::table& t, const char* name) {
        const sol::optional<std::string> optionalColor = t[name];
        return optionalColor ? RGBA::FromString(*optionalColor) : RGBA{};
    }
    static RGBA FromString(const std::string& s) {
        // Should be on format #rrggbb or #rrggbbaa
        if (s.length() != 7 && s.length() != 9) {
            return RGBA{};
        }
        if (s[0] != '#') {
            return RGBA{};
        }
        RGBA c{};
        bool ok =
            FromChars(s[1], s[2], c.r) && FromChars(s[3], s[4], c.g) && FromChars(s[5], s[6], c.b);
        if (!ok) {
            return RGBA{};
        }
        if (s.length() == 7) {
            c.a = 1;
            return c;
        }
        if (!FromChars(s[7], s[8], c.a)) {
            return RGBA{};
        }
        return c;
    }
    double r;
    double g;
    double b;
    double a;

   private:
    static bool FromChar(const char c, uint8_t& n) {
        if (c >= '0' && c <= '9') {
            n = c - '0';
            return true;
        }
        if (c >= 'a' && c <= 'f') {
            n = c - 'a' + 10;
            return true;
        }
        if (c >= 'A' && c <= 'F') {
            n = c - 'a' + 10;
            return true;
        }
        return false;
    }
    static bool FromChars(const char c1, const char c2, double& c) {
        uint8_t n1 = 0, n2 = 0;
        if (!FromChar(c1, n1) || !FromChar(c2, n2)) {
            return false;
        }
        c = ((n1 << 4) | n2) / 255.0;
        return true;
    }
};

struct Border {
    static Border FromProperty(const sol::table& t, const char* name) {
        const sol::optional<sol::table> optionalBorder = t[name];
        return optionalBorder ? Border::FromTable(*optionalBorder) : Border{};
    }
    static Border FromTable(const sol::table& t) {
        return Border{.color = RGBA::FromProperty(t, "color"),
                      .width = GetIntProperty(t, "width", 0)};
    }
    RGBA color;
    int width;
};

struct Renderable {
    virtual ~Renderable() {}
    virtual void Compute(cairo_t* cr) {}
    virtual void Draw(cairo_t* cr, int x, int y) const {}
    Size computed;
};

static void LogComputed(const Size& computed, const char* s) {
    spdlog::trace("Computed {}: {}x{}", s, computed.cx, computed.cy);
}

static void LogDraw(const char* s, int x, int y) { spdlog::trace("Draw {}: {},{}", s, x, y); }

struct Markup : public Renderable {
    Markup(const std::string& string) : string(string), m_layout(nullptr) {}
    virtual ~Markup() {
        if (m_layout) g_object_unref(m_layout);
    }
    void Compute(cairo_t* cr) override {
        // pango_layout_set_width(m_layout, m_config.cx * PANGO_SCALE);
        // pango_layout_set_height(m_layout, m_config.cy * PANGO_SCALE);
        m_layout = pango_cairo_create_layout(cr);
        pango_layout_set_markup(m_layout, string.c_str(), -1);
        PangoRectangle rect;
        pango_layout_get_extents(m_layout, nullptr, &rect);
        pango_extents_to_pixels(&rect, nullptr);
        computed.cx = rect.width;
        computed.cy = rect.height;
        LogComputed(computed, ("Markup " + string).c_str());
    }

    void Draw(cairo_t* cr, int x, int y) const override {
        LogDraw("Markup", x, y);
        cairo_move_to(cr, x, y);
        pango_cairo_show_layout(cr, m_layout);
    }

   private:
    const std::string string;
    PangoLayout* m_layout;
};

static void BeginRectangleSubPath(cairo_t* cr, int x, int y, int cx, int cy, int radius) {
    constexpr double degrees = M_PI / 180.0;
    cairo_new_sub_path(cr);
    // A-----B
    // |     |
    // C-----D
    // B
    cairo_arc(cr, x + cx - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    // D
    cairo_arc(cr, x + cx - radius, y + cy - radius, radius, 0 * degrees, 90 * degrees);
    // C
    cairo_arc(cr, x + radius, y + cy - radius, radius, 90 * degrees, 180 * degrees);
    // A
    cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
}

struct MarkupBox : public Renderable {
    MarkupBox(const std::string& string)
        : markup(string), color({}), border({}), radius(0), padding({}) {}
    static std::unique_ptr<MarkupBox> FromTable(const sol::table& t) {
        const sol::optional<std::string> optionalMarkup = t["markup"];
        const std::string markup = optionalMarkup ? *optionalMarkup : "";
        auto box = std::make_unique<MarkupBox>(markup);
        box->radius = GetIntProperty(t, "radius", 0);
        box->border = Border::FromProperty(t, "border");
        box->color = RGBA::FromProperty(t, "color");
        box->padding = Padding::FromProperty(t, "padding");
        return box;
    }
    void Compute(cairo_t* cr) override {
        markup.Compute(cr);
        computed = markup.computed;
        computed.cx += padding.left + padding.right + (2 * border.width);
        computed.cy += padding.top + padding.bottom + (2 * border.width);
        LogComputed(computed, "MarkupBox ");
    }
    void Draw(cairo_t* cr, int x, int y) const override {
        LogDraw("MarkupBox", x, y);
        BeginRectangleSubPath(cr, x + border.width, y + border.width,
                              computed.cx - (2 * border.width), computed.cy - (2 * border.width),
                              radius);
        // Border
        if (border.width) {
            // Cairo draws lines with half of the line width within the edge and the
            // other half outside. We want everything on the outside.
            cairo_push_group_with_content(cr, CAIRO_CONTENT_ALPHA);
            cairo_set_line_width(cr, border.width * 2.0);
            cairo_set_source_rgba(cr, 0, 0, 0, 1);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_stroke_preserve(cr);
            cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
            cairo_fill_preserve(cr);
            auto mask = cairo_pop_group(cr);
            cairo_set_source_rgba(cr, border.color.r, border.color.g, border.color.b,
                                  border.color.a);
            cairo_mask(cr, mask);
            cairo_pattern_destroy(mask);
        }
        // Fill
        cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
        cairo_fill(cr);
        // Inner
        markup.Draw(cr, x + padding.left + border.width, y + padding.top + border.width);
    }

   private:
    Markup markup;
    RGBA color;
    Border border;
    uint8_t radius;
    Padding padding;
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
        // TODO: Log, report
        if (!f.isColumn && *direction != "row") return nullptr;
        f.padding = Padding::FromProperty(t, "padding");
        sol::optional<sol::table> children = t["items"];
        if (children) {
            FromChildTable(*children, f.children);
        }
        return std::unique_ptr<Renderable>(new FlexContainer(std::move(f)));
    }
    void Compute(cairo_t* cr) override {
        computed.cx = padding.left + padding.right;
        computed.cy = padding.top + padding.bottom;
        for (const auto& r : children) {
            r->Compute(cr);
            if (isColumn) {
                computed.cy += r->computed.cy;
                computed.cx = std::max(computed.cx, r->computed.cx);
            } else {
                computed.cx += r->computed.cx;
                computed.cy = std::max(computed.cy, r->computed.cy);
            }
        }
        LogComputed(computed, "FlexContainer");
    }
    void Draw(cairo_t* cr, int x, int y) const override {
        LogDraw("FlexBox", x, y);
        if (isColumn) {
            for (const auto& r : children) {
                y += padding.top;
                r->Draw(cr, x + padding.left, y);
                y += r->computed.cy + padding.bottom;
            }
        } else {
            y += padding.top;
            for (const auto& r : children) {
                x += padding.left;
                r->Draw(cr, x, y);
                x += r->computed.cx + padding.right;
            }
            y += padding.bottom;
        }
    }

   private:
    bool isColumn;
    Padding padding;
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
    if (*type == "box") {
        return MarkupBox::FromTable(t);
    }
    return nullptr;
}

std::unique_ptr<Panel> Panel::Create(std::shared_ptr<BufferPool> bufferPool,
                                     Configuration::Panel panelConfig) {
    return std::unique_ptr<Panel>(new Panel(bufferPool, panelConfig));
}

bool Panel::IsDirty(const Sources& sources) const {
    for (auto& widget : m_panelConfig.widgets) {
        for (const auto& sourceName : widget.sources) {
            if (sources.IsDirty(sourceName)) return true;
        }
    }
    return false;
}

void Panel::Draw(Output& output) {
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
    int y = 0;
    for (auto& widgetConfig : m_panelConfig.widgets) {
        cairo_save(cr);
        sol::optional<sol::object> renderOutput = widgetConfig.render(output.name);
        if (!renderOutput) {
            spdlog::error("Bad return from widget");
            continue;
        }
        auto item = FromObject(*renderOutput);
        if (!item) {
            continue;
        }
        item->Compute(cr);
        auto widgetCx = item->computed.cx;
        y += widgetConfig.padding.top;
        int x = alignRight ? bufferCx - widgetCx - widgetConfig.padding.right
                           : widgetConfig.padding.left;
        item->Draw(cr, x, y);
        cairo_restore(cr);
        y += item->computed.cy + widgetConfig.padding.bottom;
    }
    // TODO: Exact x,y and cx, cy
    auto size = Size{(uint32_t)bufferCx, (uint32_t)y};
    output.Draw(m_panelConfig.index, m_panelConfig.anchor, *buffer, size);
}
