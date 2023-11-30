
#include "src/ScriptContext.h"

#include "sol/sol.hpp"
#include "spdlog/spdlog.h"

int GetIntProperty(const sol::table& t, const char* name, int missing) {
    const sol::optional<int> o = t[name];
    return o ? *o : missing;
}

static RGBA RGBAFromProperty(const sol::table& t, const char* name) {
    const sol::optional<std::string> optionalColor = t[name];
    return optionalColor ? RGBA::FromString(*optionalColor) : RGBA{};
}

static Border BorderFromTable(const sol::table& t) {
    return Border{.color = RGBAFromProperty(t, "color"),
                  .width = GetIntProperty(t, "width", 0)};
}

static Border BorderFromProperty(const sol::table& t, const char* name) {
    const sol::optional<sol::table> optionalBorder = t[name];
    return optionalBorder ? BorderFromTable(*optionalBorder) : Border{};
}

Padding PaddingFromTable(const sol::table& t) {
    return Padding{.left = GetIntProperty(t, "left", 0),
                   .right = GetIntProperty(t, "right", 0),
                   .top = GetIntProperty(t, "top", 0),
                   .bottom = GetIntProperty(t, "bottom", 0)};
}

Padding PaddingFromProperty(const sol::table& t, const char* name) {
    const sol::optional<sol::table> o = t[name];
    return o ? PaddingFromTable(*o) : Padding{};
}

static std::unique_ptr<MarkupBox> MarkupBoxFromTable(const sol::table& t) {
    const sol::optional<std::string> optionalMarkup = t["markup"];
    const std::string markup = optionalMarkup ? *optionalMarkup : "";
    auto box = std::make_unique<MarkupBox>(markup);
    box->radius = GetIntProperty(t, "radius", 0);
    box->border = BorderFromProperty(t, "border");
    box->color = RGBAFromProperty(t, "color");
    box->padding = PaddingFromProperty(t, "padding");
    return box;
}

static std::unique_ptr<Renderable> FromObject(const sol::object& o);
static void FromChildTable(const sol::table childTable,
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

static std::unique_ptr<Renderable> FlexContainerFromTable(const sol::table& t) {
    FlexContainer f;
    const sol::optional<std::string> direction = t["direction"];
    f.isColumn = direction ? *direction == "column" : true;
    // TODO: Log, report
    if (!f.isColumn && *direction != "row") return nullptr;
    f.padding = PaddingFromProperty(t, "padding");
    sol::optional<sol::table> children = t["items"];
    if (children) {
        FromChildTable(*children, f.children);
    }
    return std::unique_ptr<Renderable>(new FlexContainer(std::move(f)));
}

static std::unique_ptr<Renderable> FromObject(const sol::object& o) {
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
        return FlexContainerFromTable(t);
    }
    if (*type == "box") {
        return MarkupBoxFromTable(t);
    }
    return nullptr;
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

static void ParseWidgetConfig(const sol::table& table,
                              std::vector<WidgetConfig>& widgets) {
    WidgetConfig widget;
    widget.sources = ParseSources(table);
    sol::optional<sol::function> maybeRenderFunction = table["on_render"];
    if (!maybeRenderFunction) {
        // TODO: Log
        return;
    }
    auto renderFunction = *maybeRenderFunction;
    widget.render = [renderFunction](auto outputName) { return FromObject(renderFunction(outputName)); };
    sol::optional<sol::function> maybeClickFunction = table["on_click"];
    if (maybeClickFunction) {
      auto clickFunction = *maybeClickFunction;
      widget.click = [clickFunction]() { clickFunction(); };
    }
    widget.padding = PaddingFromProperty(table, "padding");
    widgets.push_back(std::move(widget));
}

static PanelConfig ParsePanelConfig(const sol::table panelTable, int index) {
    auto panel = PanelConfig{};
    if (!panelTable) {
        return panel;
    }
    const sol::optional<std::string> anchorString = panelTable["anchor"];
    panel.index = index;
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
    const sol::optional<std::string> directionString = panelTable["direction"];
    panel.isColumn = !directionString || *directionString != "row";

    sol::optional<sol::function> optionalCheckDisplay = panelTable["on_display"];
    if (optionalCheckDisplay) {
        auto checkDisplay = *optionalCheckDisplay;
         panel.checkDisplay = [checkDisplay](auto outputName) { return checkDisplay(outputName); };
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
        ParseWidgetConfig(*widgetTable, panel.widgets);
    }
    return panel;
}

class ScriptContextImpl : public ScriptContext {
   public:
    ScriptContextImpl(sol::state&& lua) : m_lua(std::move(lua)) {}
    std::shared_ptr<Configuration> Execute(const char* path) override;
    void RegisterSource(std::string_view name)  override;
    void Publish(const Displays& displays) override;
    void Publish(const PowerState& power) override;
    void Publish(const AudioState& audio) override;
    void Publish(const KeyboardState& keyboard) override;
    void Publish(const Networks& networks) override;

   private:
    sol::state m_lua;
};

static std::shared_ptr<Configuration> ParseConfig(sol::optional<sol::table> root) {
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
        auto panel = ParsePanelConfig(*panelTable, i);
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
 void ScriptContextImpl::RegisterSource(std::string_view name) {
  m_lua["zen"][name] = m_lua.create_table();
}

std::shared_ptr<Configuration> ScriptContextImpl::Execute(const char* path) {
    try {
        sol::optional<sol::table> configTable = m_lua.script_file(path);
        return ParseConfig(configTable);
    } catch (const sol::error& e) {
        spdlog::error("Failed to  execute configuration file: {}", e.what());
        return nullptr;
    }
}

void ScriptContextImpl::Publish(const Displays& displays) {
    auto displaysTable = m_lua.create_table();
    for (const auto& display : displays) {
        auto displayTable = m_lua.create_table();
        auto workspacesTable = m_lua.create_table();
        for (const auto& workspace : display.workspaces) {
            auto workspaceTable = m_lua.create_table();
            auto applicationsTable = m_lua.create_table();
            for (const auto& application : workspace.applications) {
                auto applicationTable = m_lua.create_table();
                applicationTable["name"] = application.name;
                applicationTable["focus"] = application.isFocused;
                applicationTable["appid"] = application.appId;
                applicationsTable.add(applicationTable);
            }
            workspaceTable["name"] = workspace.name;
            workspaceTable["focus"] = workspace.isFocused;
            workspaceTable["applications"] = applicationsTable;
            workspacesTable.add(workspaceTable);
        }
        displayTable["workspaces"] = workspacesTable;
        spdlog::debug("Display {} focus: {}", display.name, display.isFocused);
        displayTable["focus"] = display.isFocused;
        displaysTable[display.name] = displayTable;
    }
    m_lua["zen"]["displays"] = displaysTable;
}

void ScriptContextImpl::Publish(const PowerState& power) {
    auto table = m_lua["zen"]["power"];
    table["isCharging"] = power.IsCharging;
    table["isPluggedIn"] = power.IsPluggedIn;
    table["capacity"] = (int)power.Capacity;
}

void ScriptContextImpl::Publish(const AudioState& audio) {
    auto table = m_lua["zen"]["audio"];
    table["muted"] = audio.Muted;
    table["volume"] = audio.Volume;
    table["port"] = audio.PortType;
}

void ScriptContextImpl::Publish(const KeyboardState& keyboard) {
    auto table = m_lua["zen"]["keyboard"];
    table["layout"] = keyboard.layout;
}
void ScriptContextImpl::Publish(const Networks& networks) {
    auto networksTable = m_lua.create_table();
    for (const auto& network : networks) {
        auto networkTable = m_lua.create_table();
        networkTable["up"] = network.isUp;
        networkTable["interface"] = network.interface;
        networkTable["address"] = network.address;
        networksTable.add(networkTable);
    }
    m_lua["zen"]["networks"] = networksTable;
}

std::unique_ptr<ScriptContext> ScriptContext::Create() {
    sol::state lua;
    lua.open_libraries();
    // Expose root api for configuration to Lua
    lua["zen"] = lua.create_table("zen");
    return std::unique_ptr<ScriptContext>(new ScriptContextImpl(std::move(lua)));
}
