#pragma once

#include <map>
#include <memory>

#include "src/Configuration.h"

// DO NOT expose sol2 types here, they should be kept in .cpp file
// Reason for above is that sol2 sometimes messes with code formatter/LSP in
// a real bad way causing file corruption. Also, sol2 is quite big to
// include in multiple places.
//
// This file handles all parsing from Lua into native types as well
// as converting from native types to Lua.

struct Application {
    std::string name;
    std::string appId;
    bool isFocused;
    bool isAlerted;
};

struct Workspace {
    Workspace(const std::string name_) : name(name_), isFocused(false), isAlerted(false) {}
    std::string name;
    bool isFocused;  // Has the focused application
    bool isAlerted;  // Has an alerted application
    std::vector<Application> applications;
};

struct Display {
    Display(const std::string& name_) : name(name_), isFocused(false), isAlerted(false) {}
    std::string name;
    bool isFocused;  // Has the focused workspace
    bool isAlerted;  // Has an alerted workspace (or application)
    std::vector<Workspace> workspaces;
};

using Displays = std::vector<Display>;

struct PowerState {
    bool IsAlerted;
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
    bool isAlerted;
    bool isUp;
    std::string address;
    // type (eth, wifi)
    // wifi strength

    auto operator<=>(const NetworkState& other) const = default;
};
using Networks = std::map<std::string, NetworkState>;

class ScriptContext {
   public:
    ScriptContext() {}
    virtual ~ScriptContext() {}
    static std::unique_ptr<ScriptContext> Create();
    virtual std::shared_ptr<Configuration> Execute(const char* path) = 0;
    virtual void Publish(const std::string_view name, const Displays& displays) = 0;
    virtual void Publish(const std::string_view name, const PowerState& power) = 0;
    virtual void Publish(const std::string_view name, const AudioState& audio) = 0;
    virtual void Publish(const std::string_view name, const KeyboardState& keyboard) = 0;
    virtual void Publish(const std::string_view name, const Networks& networks) = 0;
};
