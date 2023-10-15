
#include "src/ScriptContext.h"

std::unique_ptr<ScriptContext> ScriptContext::Create() {
    sol::state lua;
    lua.open_libraries();
    // Expose api for configuration to Lua
    auto zen = lua.create_table("zen");
    lua["zen"] = zen;
    zen["panels"] = lua.create_table();
    zen["sources"] = lua.create_table();
    return std::unique_ptr<ScriptContext>(new ScriptContext(std::move(lua)));
}

void ScriptContext::ExecuteFile(const char* file) { m_lua.script_file(file); }

void ScriptContext::RegisterSource(std::string_view name) {
    m_lua["zen"]["sources"][name] = m_lua.create_table();
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
                applicationTable["next"] = application.isNextFocused;
                applicationsTable.add(applicationTable);
            }
            workspaceTable["name"] = workspace.name;
            workspaceTable["focus"] = workspace.isFocused;
            workspaceTable["applications"] = applicationsTable;
            workspacesTable.add(workspaceTable);
        }
        displayTable["workspaces"] = workspacesTable;
        displaysTable[display.name] = displayTable;
    }
    m_lua["zen"]["sources"]["displays"] = displaysTable;
}

void ScriptContext::Publish(const PowerState& power) {
    auto table = m_lua["zen"]["sources"]["power"];
    table["isCharging"] = power.IsCharging;
    table["capacity"] = (int)power.Capacity;
}
void ScriptContext::Publish(const AudioState& audio) {
    auto table = m_lua["zen"]["sources"]["audio"];
    table["muted"] = audio.Muted;
    table["volume"] = audio.Volume;
}
