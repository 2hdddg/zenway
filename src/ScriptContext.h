#pragma once

#include <memory>
#include <sol/sol.hpp>

struct Application {
    std::string name;
    std::string appId;
    bool isFocused;
};

struct Workspace {
    std::string name;
    bool isFocused;  // Has the focused application
    std::vector<Application> applications;
};

struct Display {
    std::string name;
    bool isFocused;  // Has the focused workspace
    std::vector<Workspace> workspaces;
};

using Displays = std::vector<Display>;

struct PowerState {
    bool IsPluggedIn;
    bool IsCharging;
    uint8_t Capacity;

    auto operator<=>(const PowerState& other) const = default;
};

struct AudioState {
    bool Muted;
    float Volume;
    std::string PortType;

    auto operator<=>(const AudioState& other) const = default;
};

struct KeyboardState {
    std::string layout;

    auto operator<=>(const KeyboardState& other) const = default;
};

struct NetworkState {
    bool isUp;
    std::string interface;
    std::string address;
    // type (eth, wifi)
    // wifi strength

    auto operator<=>(const NetworkState& other) const = default;
};
using Networks = std::vector<NetworkState>;

class ScriptContext {
   public:
    static std::unique_ptr<ScriptContext> Create();
    sol::optional<sol::table> ExecuteFile(const char* file);
    void RegisterSource(std::string_view name);
    void Publish(const Displays& displays);
    void Publish(const PowerState& power);
    void Publish(const AudioState& audio);
    void Publish(const KeyboardState& keyboard);
    void Publish(const Networks& networks);

   private:
    ScriptContext(sol::state&& lua) : m_lua(std::move(lua)) {}
    sol::state m_lua;
};
