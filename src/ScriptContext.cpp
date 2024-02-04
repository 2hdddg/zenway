
#include "src/ScriptContext.h"

#include "sol/sol.hpp"
#include "spdlog/spdlog.h"
#include "util.h"

int GetIntProperty(const sol::table& t, const char* name, int missing) {
    const sol::optional<int> o = t[name];
    return o ? *o : missing;
}

static RGBA RGBAFromProperty(const sol::table& t, const char* name) {
    const sol::optional<std::string> optionalColor = t[name];
    return optionalColor ? RGBA::FromString(*optionalColor) : RGBA{};
}

static Border BorderFromTable(const sol::table& t) {
    return Border{.color = RGBAFromProperty(t, "color"), .width = GetIntProperty(t, "width", 0)};
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

std::string TagFromTable(const sol::table& t) {
    const sol::optional<std::string> optionalTag = t["tag"];
    return optionalTag ? *optionalTag : "";
}

static std::unique_ptr<MarkupBox> MarkupBoxFromTable(const sol::table& t) {
    const sol::optional<std::string> optionalMarkup = t["markup"];
    const std::string markup = optionalMarkup ? *optionalMarkup : "";
    auto box = std::make_unique<MarkupBox>(markup);
    box->radius = GetIntProperty(t, "radius", 0);
    box->border = BorderFromProperty(t, "border");
    box->color = RGBAFromProperty(t, "color");
    box->padding = PaddingFromProperty(t, "padding");
    box->tag = TagFromTable(t);
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
    f.tag = TagFromTable(t);
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

static void ParseWidgetConfig(const sol::table& table, std::vector<WidgetConfig>& widgets) {
    WidgetConfig widget;
    widget.sources = ParseSources(table);
    sol::optional<sol::protected_function> maybeRenderFunction = table["on_render"];
    if (!maybeRenderFunction) {
        // TODO: Log
        return;
    }
    auto renderFunction = *maybeRenderFunction;
    widget.render = [renderFunction](auto outputName) {
        sol::optional<sol::object> result = renderFunction(outputName);
        if (!result) {
            spdlog::error("Bad return from render function");
            return std::unique_ptr<Renderable>(nullptr);
        }
        return FromObject(*result);
    };
    // Click handler
    sol::optional<sol::protected_function> maybeClickFunction = table["on_click"];
    if (maybeClickFunction) {
        auto clickFunction = *maybeClickFunction;
        widget.click = [clickFunction](std::string_view tag) {
            clickFunction(tag);
            return true;
        };
    }
    // Wheel handler
    sol::optional<sol::protected_function> maybeWheelFunction = table["on_wheel"];
    if (maybeWheelFunction) {
        auto wheelFunction = *maybeWheelFunction;
        widget.wheel = [wheelFunction](std::string_view tag, int value) {
            wheelFunction(tag, value);
            return true;
        };
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
        } else if (*anchorString == "topleft") {
            panel.anchor = Anchor::TopLeft;
        } else if (*anchorString == "topright") {
            panel.anchor = Anchor::TopRight;
        } else if (*anchorString == "bottom") {
            panel.anchor = Anchor::Bottom;
        } else if (*anchorString == "bottomleft") {
            panel.anchor = Anchor::BottomLeft;
        } else if (*anchorString == "bottomright") {
            panel.anchor = Anchor::BottomRight;
        } else if (*anchorString == "center") {
            panel.anchor = Anchor::Center;
        } else {
            spdlog::error("Invalid anchor: {}", *anchorString);
        }
    }
    const sol::optional<std::string> directionString = panelTable["direction"];
    panel.isColumn = !directionString || *directionString != "row";

    sol::optional<sol::protected_function> optionalCheckDisplay = panelTable["on_display"];
    if (optionalCheckDisplay) {
        auto checkDisplay = *optionalCheckDisplay;
        panel.checkDisplay = [checkDisplay](auto outputName) {
            sol::optional<bool> b = checkDisplay(outputName);
            if (b) {
                return *b;
            }
            spdlog::error("Lua display check failed, defaulting to true");
            return true;
        };
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
    void Publish(const std::string_view name, const Displays& displays) override;
    void Publish(const std::string_view name, const PowerState& power) override;
    void Publish(const std::string_view name, const AudioState& audio) override;
    void Publish(const std::string_view name, const KeyboardState& keyboard) override;
    void Publish(const std::string_view name, const Networks& networks) override;

   private:
    sol::state m_lua;
};

static DisplaysConfig ParseDisplays(sol::optional<sol::table> sourcesTable) {
    auto config = DisplaysConfig{.compositor = Compositor::Sway};
    if (!sourcesTable) {
        return config;
    }
    auto table = sourcesTable->get<sol::optional<sol::table>>("displays");
    if (!table) {
        return config;
    }
    auto compositor = table->get_or<std::string>("compositor", "sway");
    if (compositor != "sway") {
        spdlog::error("Unknown compositor: {}", compositor);
    }
    return config;
}

static AudioConfig ParseAudio(sol::optional<sol::table> sourcesTable) {
    auto config = AudioConfig{.soundServer = SoundServer::PulseAudio};
    if (!sourcesTable) {
        return config;
    }
    auto table = sourcesTable->get<sol::optional<sol::table>>("audio");
    if (!table) {
        return config;
    }
    auto server = table->get_or<std::string>("server", "pulseaudio");
    if (server != "pulseaudio") {
        spdlog::error("Unknown sound server: {}", server);
    }
    return config;
}

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
    // Alert panel. Reserve index -1 for alert
    std::optional<sol::table> alertPanelTable = (*root)["alert"];
    if (alertPanelTable) {
        config->alertPanel = ParsePanelConfig(*alertPanelTable, -1);
    } else {
        config->alertPanel = PanelConfig{.widgets = {},
                                         .index = -1,
                                         .anchor = Anchor::Center,
                                         .isColumn = false,
                                         .checkDisplay = nullptr};
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
    // Sources
    auto sources = root->get<sol::optional<sol::table>>("sources");
    config->displays = ParseDisplays(sources);
    config->audio = ParseAudio(sources);
    return config;
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

void ScriptContextImpl::Publish(const std::string_view name, const Displays& displays) {
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
                applicationTable["alert"] = application.isAlerted;
                applicationTable["appid"] = application.appId;
                applicationsTable.add(applicationTable);
            }
            workspaceTable["name"] = workspace.name;
            workspaceTable["focus"] = workspace.isFocused;
            workspaceTable["alert"] = workspace.isAlerted;
            workspaceTable["applications"] = applicationsTable;
            workspacesTable.add(workspaceTable);
        }
        displayTable["workspaces"] = workspacesTable;
        spdlog::debug("Display {} focus: {}", display.name, display.isFocused);
        displayTable["focus"] = display.isFocused;
        displayTable["alert"] = display.isAlerted;
        displaysTable[display.name] = displayTable;
    }
    m_lua["zen"][name] = displaysTable;
}

void ScriptContextImpl::Publish(const std::string_view name, const PowerState& power) {
    auto table = m_lua.create_table();
    table["isAlerted"] = power.IsAlerted;
    table["isCharging"] = power.IsCharging;
    table["isPluggedIn"] = power.IsPluggedIn;
    table["capacity"] = (int)power.Capacity;
    m_lua["zen"][name] = table;
}

void ScriptContextImpl::Publish(const std::string_view name, const AudioState& audio) {
    auto table = m_lua.create_table();
    table["muted"] = audio.Muted;
    table["volume"] = audio.Volume;
    table["port"] = audio.PortType;
    m_lua["zen"][name] = table;
}

void ScriptContextImpl::Publish(const std::string_view name, const KeyboardState& keyboard) {
    auto table = m_lua.create_table();
    table["layout"] = keyboard.layout;
    m_lua["zen"][name] = table;
}
void ScriptContextImpl::Publish(const std::string_view name, const Networks& networks) {
    auto networksTable = m_lua.create_table();
    for (const auto& network : networks) {
        auto networkTable = m_lua.create_table();
        networkTable["up"] = network.isUp;
        networkTable["interface"] = network.interface;
        networkTable["address"] = network.address;
        networksTable.add(networkTable);
    }
    m_lua["zen"][name] = networksTable;
}

std::string HtmlEscape(sol::optional<std::string> maybeString) {
    if (!maybeString) {
        spdlog::error("html_encode requires string");
        return "";
    }
    return Util::HtmlEscape(*maybeString);
}

std::unique_ptr<ScriptContext> ScriptContext::Create() {
    sol::state lua;
    lua.open_libraries();
    // Build utilities
    auto util = lua.create_table();
    util.set_function("html_escape", &HtmlEscape);
    auto zen = lua.create_table();
    zen["u"] = util;
    // Expose root api for configuration to Lua
    lua["zen"] = zen;
    return std::unique_ptr<ScriptContext>(new ScriptContextImpl(std::move(lua)));
}
