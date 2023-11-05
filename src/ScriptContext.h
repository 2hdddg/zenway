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

    bool operator==(const AudioState& other) {
        return Muted == other.Muted && Volume == other.Volume;
    }
};

struct KeyboardState {
    std::string layout;
};

class ScriptContext {
   public:
    static std::unique_ptr<ScriptContext> Create();
    void ExecuteFile(const char* file);
    void RegisterSource(std::string_view name);
    sol::table Root() { return m_lua["zen"]; }
    void Publish(const Displays& displays);
    void Publish(const PowerState& power);
    void Publish(const AudioState& audio);
    void Publish(const KeyboardState& keyboard);

   private:
    ScriptContext(sol::state&& lua) : m_lua(std::move(lua)) {}
    sol::state m_lua;
};
