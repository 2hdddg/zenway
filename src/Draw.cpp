#include "Draw.h"

#include "pango/pango-layout.h"
#include "pango/pangocairo.h"
#include "spdlog/spdlog.h"

static void LogComputed(const Size& computed, const char* s) {
    spdlog::trace("Computed {}: {}x{}", s, computed.cx, computed.cy);
}

static void LogDraw(const char* s, int x, int y) { spdlog::trace("Draw {}: {},{}", s, x, y); }

void Markup::Compute(cairo_t* cr) {
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

void Markup::Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const {
    LogDraw("Markup", x, y);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, m_layout);
}

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

void MarkupBox::Compute(cairo_t* cr) {
    markup.Compute(cr);
    computed = markup.computed;
    computed.cx += padding.left + padding.right + (2 * border.width);
    computed.cy += padding.top + padding.bottom + (2 * border.width);
    LogComputed(computed, "MarkupBox ");
}

void MarkupBox::Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const {
    LogDraw("MarkupBox", x, y);
    BeginRectangleSubPath(cr, x + border.width, y + border.width, computed.cx - (2 * border.width),
                          computed.cy - (2 * border.width), radius);
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
        cairo_set_source_rgba(cr, border.color.r, border.color.g, border.color.b, border.color.a);
        cairo_mask(cr, mask);
        cairo_pattern_destroy(mask);
    }
    // Fill
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    cairo_fill(cr);
    // Inner
    markup.Draw(cr, x + padding.left + border.width, y + padding.top + border.width, targets);
    if (tag != "") {
        targets.push_back(Target{
            .position = Rect{.x = x, .y = y, .cx = computed.cx, .cy = computed.cy}, .tag = tag});
    }
}

void FlexContainer::Compute(cairo_t* cr) {
    computed.cx = 0;
    computed.cy = 0;
    for (const auto& r : children) {
        r->Compute(cr);
        if (isColumn) {
            computed.cy += padding.top + padding.bottom + r->computed.cy;
            computed.cx = std::max(computed.cx, r->computed.cx + padding.left + padding.right);
        } else {
            computed.cx += padding.left + padding.right + r->computed.cx;
            computed.cy = std::max(computed.cy, r->computed.cy + padding.top + padding.bottom);
        }
    }
    LogComputed(computed, "FlexContainer");
}

void FlexContainer::Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const {
    LogDraw("FlexBox", x, y);
    const int startx = x;
    const int starty = y;
    if (isColumn) {
        for (const auto& r : children) {
            y += padding.top;
            r->Draw(cr, x + padding.left, y, targets);
            y += r->computed.cy + padding.bottom;
        }
    } else {
        y += padding.top;
        for (const auto& r : children) {
            x += padding.left;
            r->Draw(cr, x, y, targets);
            x += r->computed.cx + padding.right;
        }
        y += padding.bottom;
    }
    if (tag != "") {
        targets.push_back(
            Target{.position = Rect{.x = startx, .y = starty, .cx = computed.cx, .cy = computed.cy},
                   .tag = tag});
    }
}

void Widget::Compute(const WidgetConfig& config, const std::string& outputName, cairo_t* cr) {
    auto item = config.render(outputName);
    if (!item) {
        spdlog::error("Bad render return from widget");
        m_paddingX = 0;
        m_paddingY = 0;
        computed.cx = 0;
        computed.cy = 0;
        return;
    }
    item->Compute(cr);
    m_paddingX = config.padding.left;
    m_paddingY = config.padding.top;
    computed.cx = item->computed.cx + config.padding.left + config.padding.right + m_paddingX;
    computed.cy = item->computed.cy + config.padding.top + config.padding.bottom + m_paddingY;
    m_renderable = std::move(item);
}

void Widget::Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const {
    if (!m_renderable) {
        // Lua render failed previously
        return;
    }
    cairo_save(cr);
    m_renderable->Draw(cr, x + m_paddingX, y + m_paddingY, targets);
    cairo_restore(cr);
}

enum class Align { Left, Right, Top, Bottom, CenterX, CenterY };

bool Draw::Panel(const PanelConfig& panelConfig, const std::string& outputName,
                 BufferPool& bufferPool, DrawnPanel& drawn) {
    // Get free buffer to draw in. This could fail if both buffers are locked.
    auto buffer = bufferPool.Get();
    if (!buffer) {
        spdlog::error("No buffer to draw in");
        return false;
    }
    // Clear buffer
    // TODO: Could delay this to only clear part that will be used when drawing
    auto cr = buffer->GetCairoCtx();
    buffer->Clear(0x00);
    // Calculate size of all widgets and track max width and height
    auto widgets = std::vector<Widget>(panelConfig.widgets.size());
    int maxCx = 0, maxCy = 0;
    int cx = 0, cy = 0;
    for (size_t i = 0; i < widgets.size(); i++) {
        auto& widget = widgets[i];
        auto& config = panelConfig.widgets[i];
        widget.Compute(config, outputName, cr);
        maxCx = std::max(maxCx, widget.computed.cx);
        maxCy = std::max(maxCy, widget.computed.cy);
        cx += widget.computed.cx;
        cy += widget.computed.cy;
    }
    std::vector<Target> targets;
    Align align;
    int xfac = 0, yfac = 0;
    if (panelConfig.isColumn) {
        yfac = 1;
        cx = maxCx;
        switch (panelConfig.anchor) {
            case Anchor::Top:
            case Anchor::Bottom:
                align = Align::CenterX;
                break;
            case Anchor::Left:
                align = Align::Left;
                break;
            case Anchor::Right:
                align = Align::Right;
                break;
        }
    } else {
        xfac = 1;
        cy = maxCy;
        switch (panelConfig.anchor) {
            case Anchor::Top:
                align = Align::Top;
                break;
            case Anchor::Bottom:
                align = Align::Bottom;
                break;
            case Anchor::Left:
            case Anchor::Right:
                align = Align::CenterY;
                break;
        }
    }
    // Draw to buffer
    int x = 0, y = 0;
    for (const auto& widget : widgets) {
        switch (align) {
            case Align::Left:
                break;
            case Align::Top:
                break;
            case Align::Right:
                x = std::max((maxCx - widget.computed.cx), 0);
                break;
            case Align::Bottom:
                y = std::max((maxCy - widget.computed.cy), 0);
                break;
            case Align::CenterX:
                x = (maxCx - widget.computed.cx) / 2;
                break;
            case Align::CenterY:
                y = (maxCy - widget.computed.cy) / 2;
                break;
        }
        widget.Draw(cr, x, y, targets);
        drawn.widgets.push_back(
            DrawnWidget{.position = {x, y, widget.computed.cx, widget.computed.cy},
                        .targets = std::move(targets)});
        x += widget.computed.cx * xfac;
        y += widget.computed.cy * yfac;
    }
    drawn.size = Size{cx, cy};
    drawn.buffer = buffer;
    return true;
}
