#include "Configuration.h"

#include <spdlog/spdlog.h>

#include <set>
#include <sol/sol.hpp>

#include "src/Sources.h"

static std::set<std::string> ParseSources(const sol::table& widgetTable) {
    std::set<std::string> sources;
    const sol::optional<sol::table> table = widgetTable["sources"];
    if (!table) {
        return sources;
    }
    auto numSources = table->size();
    for (int i = 0; i < numSources; i++) {
        sources.insert(table->get<std::string>(i + 1));
    }
    return sources;
}

void Configuration::Widget::Parse(const sol::table& table, std::vector<Widget>& widgets) {
    Widget widget;
    widget.sources = ParseSources(table);
    sol::optional<sol::function> renderFunction = table["render"];
    if (!renderFunction) {
        // TODO: Log
        return;
    }
    widget.render = *renderFunction;
    widgets.push_back(std::move(widget));
}

Configuration::Panel Configuration::Panel::Parse(const sol::table table, const char* name,
                                                 int index) {
    sol::optional<sol::table> panelTable = table[name];
    Panel panel;
    panel.index = index;

    if (!panelTable) {
        return panel;
    }
    sol::optional<sol::table> widgetsTable = (*panelTable)["widgets"];
    if (!widgetsTable) {
        return panel;
    }
    auto numWidgets = widgetsTable->size();
    if (numWidgets == 0) {
        return panel;
    }
    // Parse each widget
    for (int i = 0; i < numWidgets; i++) {
        sol::optional<sol::table> widgetTable = (*widgetsTable)[i + 1];
        if (!widgetTable) {
            // TODO: Log!
            continue;
        }
        Configuration::Widget::Parse(*widgetTable, panel.widgets);
    }
    // Get offset from screen border
    sol::optional<int> screenOffset = (*panelTable)["screen_border_offset"];
    panel.screenBorderOffset = screenOffset ? *screenOffset : 0;
    return panel;
}

std::shared_ptr<Configuration> Configuration::Read(ScriptContext& scriptContext) {
    scriptContext.ExecuteFile("../test.lua");
    // "Parse" the configuration state
    sol::table panels = scriptContext.Root()["panels"];
    auto config = std::unique_ptr<Configuration>(new Configuration());
    config->leftPanel = Panel::Parse(panels, "left", 0);
    config->leftPanel.anchor = Anchor::Left;
    config->rightPanel = Panel::Parse(panels, "right", 1);
    config->rightPanel.anchor = Anchor::Right;
    return config;
}
