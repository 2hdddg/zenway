#include "Panel.h"

#include "pango/pango-layout.h"
#include "pango/pangocairo.h"
#include "src/BufferPool.h"

std::unique_ptr<Panel> Panel::Create(std::shared_ptr<BufferPool> bufferPool,
                                     Configuration::Panel panelConfig,
                                     std::shared_ptr<Sources> sources) {
    return std::unique_ptr<Panel>(new Panel(bufferPool, panelConfig, sources));
}
/*
void XWidget::Draw(PangoLayout& layout) {
    std::string markup = m_renderFunction();
    pango_layout_set_markup(&layout, markup.c_str(), -1);
}
*/

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
    PangoLayout* layout = pango_cairo_create_layout(cr);
    auto bufferCx = buffer->Cx();
    bool alignRight = m_panelConfig.anchor == Anchor::Right;

    for (auto& widget : m_panelConfig.widgets) {
        cairo_save(cr);

        // if (IsDirty(widget, *m_sources)) {
        //  widget.Draw(*layout);
        sol::object renderOutput = widget.render(output.name);
        if (renderOutput.is<std::string>()) {
            std::string markup = renderOutput.as<std::string>();
            pango_layout_set_markup(layout, markup.c_str(), -1);
        } else {
            pango_layout_set_markup(layout, "Bad", -1);
        }

        PangoRectangle rect;
        pango_layout_get_extents(layout, nullptr, &rect);
        pango_extents_to_pixels(nullptr, &rect);
        if (alignRight) {
            cairo_move_to(cr, bufferCx - rect.width - 10, y);
        } else {
            cairo_move_to(cr, 0, y);
        }
        pango_cairo_show_layout(cr, layout);
        cairo_restore(cr);
        y += rect.height;
        y += 10;
    }
    output.surfaces[m_panelConfig.index]->Draw(m_panelConfig.anchor, *buffer, 0, 0, bufferCx, y);
    g_object_unref(layout);
}
