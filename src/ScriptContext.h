#pragma once

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
     ScriptContext(){}
    virtual ~ScriptContext() {}
    static std::unique_ptr<ScriptContext> Create();
    virtual std::shared_ptr<Configuration> Execute(const char* path) = 0;
    virtual void RegisterSource(std::string_view name)  = 0;
    virtual void Publish(const Displays& displays) = 0;
    virtual void Publish(const PowerState& power) = 0;
    virtual void Publish(const AudioState& audio) = 0;
    virtual void Publish(const KeyboardState& keyboard) = 0;
    virtual void Publish(const Networks& networks) = 0;
};
