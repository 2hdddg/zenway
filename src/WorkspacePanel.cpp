#include "WorkspacePanel.h"

#include <pango/pangocairo.h>
#include <spdlog/spdlog.h>

#include "Widget.h"
#include "cairo.h"
#include "pango/pango-layout.h"

/*
std::unique_ptr<WorkspacePanel> WorkspacePanel::Create(std::shared_ptr<BufferPool> bufferPool,
                                                       MainLoop& mainLoop,
                                                       const std::shared_ptr<Theme> theme) {
    return std::unique_ptr<WorkspacePanel>(new WorkspacePanel(bufferPool, theme));
}

static std::string GetFocusedApplicationName(const Workspace& workspace) {
    for (auto& app : workspace.applications) {
        if (app.focused || app.nextFocused) {
            return app.name;
        }
    }
    return std::string();
}

void WorkspacePanel::Draw(Output& output) {
    // Widget().Draw(output, *m_bufferPool);
    // return;
    auto y = 0;
    // Get free buffer to draw in. This could fail if both buffers are locked.
    auto buffer = m_bufferPool->Get();
    if (!buffer) {
        spdlog::error("No buffer to draw in");
        return;
    }
    // Draw to buffer
    auto cr = buffer->GetCairoCtx();
    buffer->Clear(0x00);
    PangoLayout* layout = pango_cairo_create_layout(cr);

    for (const auto& workspace : output.workspaces) {
        cairo_save(cr);
        std::string focusedApplicationName = GetFocusedApplicationName(workspace);
        spdlog::info("Focused app {}", focusedApplicationName);
        auto theme =
            m_theme->ForWorkspace(workspace.focused, workspace.name, focusedApplicationName);
        pango_layout_set_markup(layout, theme.Markup.c_str(), -1);
        PangoRectangle rect;
        pango_layout_get_extents(layout, nullptr, &rect);
        pango_extents_to_pixels(nullptr, &rect);
        cairo_move_to(cr, theme.OffsetX, y);
        pango_cairo_show_layout(cr, layout);
        cairo_restore(cr);
        y += rect.height;
        y += theme.SpacingY;
    }
    output.surfaces[0]->Draw(Anchor::Left, *buffer, 0, 0, buffer->Cx(), y);
    // output.Draw(Output::Panel::Workspace, Anchor::Left, *buffer, 0, 0, buffer->Cx(), y);
    g_object_unref(layout);
}
*/
