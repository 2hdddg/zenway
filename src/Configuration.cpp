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
    for (size_t i = 0; i < numSources; i++) {
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

Configuration::Panel Configuration::Panel::Parse(const sol::table panelTable, int index) {
    Panel panel;
    panel.index = index;

    if (!panelTable) {
        return panel;
    }
    const sol::optional<std::string> anchorString = panelTable["anchor"];
    panel.anchor = Anchor::Left;
    if (anchorString) {
        if (*anchorString == "left") {
        } else if (*anchorString == "right") {
            panel.anchor = Anchor::Right;
        } else if (*anchorString == "top") {
            panel.anchor = Anchor::Top;
        } else if (*anchorString == "bottom") {
            panel.anchor = Anchor::Bottom;
        } else {
            spdlog::error("Invalid anchor: {}", *anchorString);
        }
    }

    sol::optional<sol::table> widgetsTable = panelTable["widgets"];
    if (!widgetsTable) {
        return panel;
    }
    auto numWidgets = widgetsTable->size();
    if (numWidgets == 0) {
        return panel;
    }
    // Parse each widget
    for (size_t i = 0; i < numWidgets; i++) {
        sol::optional<sol::table> widgetTable = (*widgetsTable)[i + 1];
        if (!widgetTable) {
            // TODO: Log!
            continue;
        }
        Configuration::Widget::Parse(*widgetTable, panel.widgets);
    }
    return panel;
}

std::shared_ptr<Configuration> Configuration::Read(ScriptContext& scriptContext, const char* file) {
    auto root = scriptContext.ExecuteFile(file);
    if (!root) return nullptr;
    // "Parse" the configuration state
    sol::optional<sol::table> panelsTable = (*root)["panels"];
    if (!panelsTable) {
        spdlog::error("No panels found");
        return nullptr;
    }
    auto config = std::unique_ptr<Configuration>(new Configuration());
    // Panels
    for (size_t i = 0; i < panelsTable->size(); i++) {
        sol::optional<sol::table> panelTable = (*panelsTable)[i + 1];
        if (!panelTable) {
            spdlog::error("Expected panel table");
            continue;
        }
        auto panel = Panel::Parse(*panelTable, i);
        config->panels.push_back(panel);
    }
    // Buffers
    sol::optional<sol::table> buffersTable = (*root)["buffers"];
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
