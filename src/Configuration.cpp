#include "Configuration.h"

#include <spdlog/spdlog.h>

#include <set>
#include <sol/sol.hpp>

#include "src/Sources.h"

int GetIntProperty(const sol::table& t, const char* name, int missing) {
    const sol::optional<int> o = t[name];
    return o ? *o : missing;
}

Padding Padding::FromProperty(const sol::table& t, const char* name) {
    const sol::optional<sol::table> o = t[name];
    return o ? Padding::FromTable(*o) : Padding{};
}

Padding Padding::FromTable(const sol::table& t) {
    return Padding{.left = GetIntProperty(t, "left", 0),
                   .right = GetIntProperty(t, "right", 0),
                   .top = GetIntProperty(t, "top", 0),
                   .bottom = GetIntProperty(t, "bottom", 0)};
}
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
    widget.padding = Padding::FromProperty(table, "padding");
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
    return panel;
}

std::shared_ptr<Configuration> Configuration::Read(ScriptContext& scriptContext) {
    scriptContext.ExecuteFile("../test.lua");
    // "Parse" the configuration state
    sol::table panels = scriptContext.Root()["panels"];
    auto config = std::unique_ptr<Configuration>(new Configuration());
    // Panels
    config->leftPanel = Panel::Parse(panels, "left", 0);
    config->leftPanel.anchor = Anchor::Left;
    config->rightPanel = Panel::Parse(panels, "right", 1);
    config->rightPanel.anchor = Anchor::Right;
    // Buffers
    sol::optional<sol::table> buffersTable = scriptContext.Root()["buffers"];
    config->numBuffers = 1;
    config->bufferWidth = 1000;
    config->bufferHeight = 1000;
    if (buffersTable) {
        config->numBuffers = GetIntProperty(*buffersTable, "num", config->numBuffers);
        config->bufferWidth = GetIntProperty(*buffersTable, "width", config->bufferWidth);
        config->bufferHeight = GetIntProperty(*buffersTable, "height", config->bufferHeight);
    }
    return config;
}
