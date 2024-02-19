#pragma once

#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "cairo.h"
#include "pango/pango-layout.h"

struct Padding {
    int left;
    int right;
    int top;
    int bottom;
};

struct Size {
    int cx;
    int cy;
};

struct Rect {
    int x;
    int y;
    int cx;
    int cy;

    bool Contains(int x_, int y_) const {
        return x_ >= x && x_ <= x + cx && y_ >= y && y_ <= y + cy;
    }
};

struct RGBA {
    double r;
    double g;
    double b;
    double a;
    static RGBA FromString(const std::string& s);
};

struct Border {
    RGBA color;
    int width;
};

enum class Anchor { Left, Right, Top, TopLeft, TopRight, Bottom, BottomLeft, BottomRight, Center };

struct Target {
    Rect position;
    std::string tag;
};

struct Renderable {
    Renderable() : computed{} {}
    virtual ~Renderable() {}
    virtual void Compute(cairo_t*) {}
    virtual void Draw(cairo_t*, int /*x*/, int /*y*/, std::vector<Target>& /*targets*/) const {}
    Size computed;
};

struct Markup : public Renderable {
    Markup(const std::string& string) : Renderable(), string(string), m_layout(nullptr) {}
    virtual ~Markup() {
        if (m_layout) g_object_unref(m_layout);
    }
    void Compute(cairo_t* cr) override;
    void Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const override;

   private:
    const std::string string;
    PangoLayout* m_layout;
};

struct MarkupBox : public Renderable {
    MarkupBox(const std::string& string)
        : Renderable(), markup(string), color({}), border({}), radius(0), padding({}) {}
    void Compute(cairo_t* cr) override;
    void Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const override;

    Markup markup;
    RGBA color;
    Border border;
    uint8_t radius;
    Padding padding;
    std::string tag;
};

struct FlexContainer : public Renderable {
    FlexContainer() : Renderable(), isColumn(false), padding({}) {}
    void Compute(cairo_t* cr) override;
    void Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const override;

    bool isColumn;
    Padding padding;
    std::vector<std::unique_ptr<Renderable>> children;
    std::string tag;
};

struct WidgetConfig {
    WidgetConfig() : padding({}) {}
    std::function<std::unique_ptr<Renderable>(const std::string& outputName)> render;
    std::function<void(std::string_view tag)> click;
    std::function<void(std::string_view tag, int value)> wheel;
    std::set<std::string> sources;
    Padding padding;
};

struct Widget {
    Widget() : computed({}), m_renderable(nullptr), m_paddingX(0), m_paddingY(0) {}
    void Compute(const WidgetConfig& config, const std::string& outputName, cairo_t* cr);
    void Draw(cairo_t* cr, int x, int y, std::vector<Target>& targets) const;
    Size computed;

   private:
    std::unique_ptr<Renderable> m_renderable;
    int m_paddingX;
    int m_paddingY;
};

struct PanelConfig {
    std::vector<WidgetConfig> widgets;
    int index;
    Anchor anchor;
    bool isColumn;
    std::function<bool(const std::string& outputName)> checkDisplay;
};

enum class Compositor {
    Sway,
};

struct DisplaysConfig {
    Compositor compositor;
};

enum class SoundServer {
    PulseAudio,
};

struct AudioConfig {
    SoundServer soundServer;
};

class Configuration {
   public:
    std::vector<PanelConfig> panels;
    PanelConfig alertPanel;
    DisplaysConfig displays;
    AudioConfig audio;
    int bufferWidth;
    int bufferHeight;
    int numBuffers;
};
