#include "Widget.h"

#include <optional>

#include "pango/pangocairo.h"

/*
struct Color {
   public:
    double r;
    double g;
    double b;
    double a;
};

struct BoxConfiguration {
   public:
    int cx;
    int cy;
    std::optional<Color> backgroundColor;
    int borderSize;
    Color borderColor;
};

class Box {
   public:
    Box(const BoxConfiguration& config, const char* markup) : m_config(config), m_markup(markup) {}
    void Prepare(cairo_t* cr) {
        m_layout = pango_cairo_create_layout(cr);
        pango_layout_set_width(m_layout, m_config.cx * PANGO_SCALE);
        pango_layout_set_height(m_layout, m_config.cy * PANGO_SCALE);
        pango_layout_set_markup(m_layout, m_markup.c_str(), -1);
        PangoRectangle rect;
        pango_layout_get_extents(m_layout, nullptr, &rect);
        pango_extents_to_pixels(nullptr, &rect);
        m_markupCx = rect.width;
        m_markupCy = rect.height;
    }
    void Show(cairo_t* cr, int x, int y) const {
        DrawBackground(cr, x, y);
        cairo_move_to(cr, x, y);
        pango_cairo_show_layout(cr, m_layout);
    }
    int Width() const { return m_markupCx; }
    int Height() const { return m_markupCy; }

   private:
    void DrawBackground(cairo_t* cr, int x, int y) const {
        if (m_config.borderSize) {
        }
        if (m_config.backgroundColor) {
            auto bg = m_config.backgroundColor.value();
            cairo_save(cr);
            cairo_set_source_rgba(cr, bg.r, bg.g, bg.b, bg.a);
            cairo_rectangle(cr, x, y, m_markupCx, m_markupCy);
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
            cairo_restore(cr);
        }
    }
    const BoxConfiguration m_config;
    const std::string m_markup;
    PangoLayout* m_layout;
    int m_markupCx;
    int m_markupCy;
};

class Boxes {
   public:
    Boxes& Add(const BoxConfiguration& config, const char* markup) {
        m_boxes.push_back(Box(config, markup));
        return *this;
    }
    void Prepare(cairo_t* cr) {
        m_height = 0;
        m_width = 0;
        for (auto& box : m_boxes) {
            box.Prepare(cr);
            m_height += box.Height();
            if (box.Width() > m_width) {
                m_width = box.Width();
            }
        }
    }
    int Height() { return m_height; }
    int Width() { return m_width; }
    void Show(cairo_t* cr, int x, int y) {
        for (auto& box : m_boxes) {
            box.Show(cr, x, y);
            y += box.Height();
        }
    }

   private:
    std::vector<Box> m_boxes;
    int m_height;
    int m_width;
};

// using Columns = std::vector<Boxes>;
class Columns {
   public:
    Columns& Add(Boxes&& boxes) {
        m_columns.push_back(boxes);
        return *this;
    }
    void Show(cairo_t* cr) {
        auto y = 0;
        auto x = 0;
        m_height = 0;
        m_width = 0;
        for (auto& boxes : m_columns) {
            boxes.Prepare(cr);
            boxes.Show(cr, x, y);
            auto height = boxes.Height();
            y += height;
            if (height > m_height) {
                m_height = height;
            }
            y = 0;
            auto width = boxes.Width();
            x += width;
            m_width += width;
        }
    }
    int Height() { return m_height; }

   private:
    int m_height;
    int m_width;
    std::vector<Boxes> m_columns;
};

void Widget::Draw(Output& output, BufferPool& pool) {
    auto buffer = pool.Get();
    auto cr = buffer->GetCairoCtx();
    buffer->Clear(0x00);

    // Test
    auto columns =
        Columns()
            .Add(std::move(Boxes()
                               .Add(BoxConfiguration{.cx = 300,
                                                     .cy = 40,
                                                     .backgroundColor = Color{.r = 0.5, .a = 1}},
                                    "<span size='15pt' color='#102010'>Testing 1 a b c</span>")
                               .Add(BoxConfiguration{.backgroundColor = Color{.b = .7, .a = .7}},
                                    "Testing 2")))
            .Add(std::move(Boxes().Add(
                BoxConfiguration{.cx = 300, .cy = 40, .backgroundColor = Color{.g = 0.5, .a = .5}},
                "<span size='15pt' color='#102010'>Column 2</span>")));
    cairo_save(cr);
    columns.Show(cr);
    cairo_restore(cr);
    output.Draw(Output::Panel::Workspace, Anchor::Left, *buffer, 0, 0, buffer->Cx(),
                columns.Height());
}
*/
