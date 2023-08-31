#pragma once

#include <memory>
#include <sol/sol.hpp>

struct Application {
    std::string name;
    std::string appId;
    bool isFocused;
    bool isNextFocused;
};

struct Workspace {
    std::string name;
    bool isFocused;
    std::vector<Application> applications;
};

struct Display {
    std::string name;
    std::vector<Workspace> workspaces;
};

using Displays = std::vector<Display>;

class ScriptContext {
   public:
    static std::unique_ptr<ScriptContext> Create();
    void ExecuteFile(const char* file);
    void RegisterSource(std::string_view name);
    sol::table Root() { return m_lua["zen"]; }
    sol::table Sources() { return m_lua["zen"]["sources"]; }
    sol::table Source(std::string_view name) { return m_lua["zen"]["sources"][name]; }
    void Publish(const Displays& displays);

   private:
    ScriptContext(sol::state&& lua) : m_lua(std::move(lua)) {}
    sol::state m_lua;
};
