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

struct PowerState {
    bool IsPluggedIn;
    bool IsCharging;
    uint8_t Capacity;

    bool operator==(const PowerState& other) {
        return IsPluggedIn == other.IsPluggedIn && IsCharging == other.IsCharging &&
               Capacity == other.Capacity;
    }
};

struct AudioState {
    bool Muted;
    float Volume;
    std::string PortType;

    bool operator==(const AudioState& other) {
        return Muted == other.Muted && Volume == other.Volume && PortType == other.PortType;
    }
};

struct KeyboardState {
    std::string layout;
};

struct NetworkState {
    bool isUp;
    std::string interface;
    std::string address;
    // type (eth, wifi)
    // wifi strength
};
using Networks = std::vector<NetworkState>;

class ScriptContext {
   public:
    static std::unique_ptr<ScriptContext> Create();
    bool ExecuteFile(const char* file);
    void InitializeRuntime();
    void RegisterSource(std::string_view name);
    sol::table Root() { return m_lua["zen"]; }
    void Publish(const Displays& displays);
    void Publish(const PowerState& power);
    void Publish(const AudioState& audio);
    void Publish(const KeyboardState& keyboard);
    void Publish(const Networks& networks);

   private:
    ScriptContext(sol::state&& lua) : m_lua(std::move(lua)) {}
    sol::state m_lua;
};
