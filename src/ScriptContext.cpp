
#include "src/ScriptContext.h"

#include "spdlog/spdlog.h"

std::unique_ptr<ScriptContext> ScriptContext::Create() {
    sol::state lua;
    lua.open_libraries();
    // Expose root api for configuration to Lua
    lua["zen"] = lua.create_table("zen");
    return std::unique_ptr<ScriptContext>(new ScriptContext(std::move(lua)));
}

sol::optional<sol::table> ScriptContext::ExecuteFile(const char* file) {
    try {
        return m_lua.script_file(file);
    } catch (const sol::error& e) {
        spdlog::error("Failed to execute configuration file: {}", e.what());
        return sol::optional<sol::table>();
    }
}

void ScriptContext::RegisterSource(std::string_view name) {
    m_lua["zen"][name] = m_lua.create_table();
}

void ScriptContext::Publish(const Displays& displays) {
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

void ScriptContext::Publish(const PowerState& power) {
    auto table = m_lua["zen"]["power"];
    table["isCharging"] = power.IsCharging;
    table["isPluggedIn"] = power.IsPluggedIn;
    table["capacity"] = (int)power.Capacity;
}

void ScriptContext::Publish(const AudioState& audio) {
    auto table = m_lua["zen"]["audio"];
    table["muted"] = audio.Muted;
    table["volume"] = audio.Volume;
    table["port"] = audio.PortType;
}

void ScriptContext::Publish(const KeyboardState& keyboard) {
    auto table = m_lua["zen"]["keyboard"];
    table["layout"] = keyboard.layout;
}
void ScriptContext::Publish(const Networks& networks) {
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
