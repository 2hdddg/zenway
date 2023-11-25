#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <cstdlib>
#include <filesystem>

#include "Buffer.h"
#include "Configuration.h"
#include "DateTimeSources.h"
#include "MainLoop.h"
#include "Manager.h"
#include "NetworkSource.h"
#include "Panel.h"
#include "PowerSource.h"
#include "PulseAudioSource.h"
#include "Registry.h"
#include "Sources.h"
#include "SwayCompositor.h"

static const std::optional<std::filesystem::path> ProbeForConfig(int argc, char* argv[]) {
    // Explicit config
    if (argc > 1) {
        return std::filesystem::path(argv[1]);
    }
    // Check user config
    std::optional<std::filesystem::path> path;
    auto xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
    if (xdgConfigHome) {
        path = std::filesystem::path(xdgConfigHome);
    } else {
        auto home = std::getenv("HOME");
        if (home) {
            path = std::filesystem::path(home);
            path->append(".config");
        }
    }
    if (path) {
        path->append("zenway");
        path->append("config.lua");
        if (std::filesystem::exists(*path)) {
            return path;
        }
    }
    return std::optional<std::filesystem::path>();
}

static void InitializeSource(const std::string& source, Sources& sources,
                             std::shared_ptr<MainLoop> mainLoop,
                             std::shared_ptr<ScriptContext> scriptContext,
                             const Registry& registry) {
    if (source == "date" || source == "time") {
        //  Date time sources
        auto dateSource = DateSource::Create();
        auto timeSource = TimeSource::Create(*mainLoop, dateSource);
        sources.Register("date", dateSource);
        sources.Register("time", timeSource);
        return;
    }
    if (source == "workspace") {
        // Initialized by manager later..
        return;
    }
    if (source == "networks") {
        auto networkSource = NetworkSource::Create(*mainLoop, scriptContext);
        if (!networkSource) {
            spdlog::error("Failed to initialize network source");
            return;
        }
        sources.Register(source, networkSource);
        networkSource->Initialize();
        return;
    }
    if (source == "audio") {
        auto audioSource = PulseAudioSource::Create(mainLoop, scriptContext);
        if (!audioSource) {
            spdlog::error("Failed to initialize PulseAudio source");
            return;
        }
        sources.Register(source, std::move(audioSource));
        return;
    }
    if (source == "power") {
        auto powerSource = PowerSource::Create(*mainLoop, scriptContext);
        if (!powerSource) {
            spdlog::error("Failed to initialize battery source");
            return;
        }
        sources.Register(source, powerSource);
        // Initialize after registration
        powerSource->Initialize();
        return;
    }
    if (source == "keyboard") {
        if (registry.seat && registry.seat->keyboard) {
            sources.Register("keyboard", registry.seat->keyboard);
            registry.seat->keyboard->SetScriptContext(scriptContext);
        } else {
            spdlog::warn("No keyboard source");
        }
        return;
    }
    spdlog::error("Unknown source: {}", source);
}

int main(int argc, char* argv[]) {
    // Environment variable configurable logging
    spdlog::cfg::load_env_levels();
    auto scriptContext = std::shared_ptr<ScriptContext>(ScriptContext::Create());
    if (!scriptContext) {
        spdlog::error("Failed to create script context");
        return -1;
    }
    // Detect location of configuration
    auto configPath = ProbeForConfig(argc, argv);
    if (!configPath) {
        spdlog::error("No configuration found");
        return -1;
    }
    spdlog::info("Loading configuration at: {}", configPath->c_str());
    // Read configuration
    auto config = Configuration::Read(*scriptContext, configPath->c_str());
    if (!config) {
        spdlog::error("Failed to read configuration");
        return -1;
    }
    std::shared_ptr<MainLoop> mainLoop = MainLoop::Create();
    if (!mainLoop) {
        spdlog::error("Failed to initialize main loop");
        return -1;
    }
    // Registry fills the outputs with output instances
    auto outputs = std::shared_ptr<Outputs>(Outputs::Create(config));
    // Initialize registry.
    // The registry initializes roots that contains elementary interfaces needed for the system
    // to work. The registry also maintains the list of active outputs (monitors).
    auto registry = Registry::Create(*mainLoop, outputs);
    if (!registry) {
        spdlog::error("Failed to initialize registry");
        return -1;
    }
    // Outputs needs roots initialized by the registry
    if (!outputs->Initialize(registry->roots)) {
        return -1;
    }
    // Initialize sources
    auto sources = Sources::Create(scriptContext);
    for (auto panelConfig : config->panels) {
        //  Check what sources are needed for the widgets in the panel
        for (const auto& widgetConfig : panelConfig.widgets) {
            for (const auto& source : widgetConfig.sources) {
                // Initialize source if not already done
                if (!sources->IsRegistered(source)) {
                    InitializeSource(source, *sources, mainLoop, scriptContext, *registry);
                }
            }
        }
    }
    // Manager handles displays and redrawing
    auto manager = Manager::Create(*mainLoop, outputs, std::move(sources));
    // Initialize compositor
    auto sway = SwayCompositor::Connect(*mainLoop, manager, scriptContext);
    if (!sway) {
        spdlog::error("Failed to connect to Sway");
        return -1;
    }
    // Leave the rest to main loop
    mainLoop->Run();
    return 0;
}
