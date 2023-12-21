#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include <filesystem>

#include "DateTimeSources.h"
#include "MainLoop.h"
#include "Manager.h"
#include "NetworkSource.h"
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
                             std::shared_ptr<ScriptContext> scriptContext, const Registry& registry,
                             const Configuration config) {
    if (source == "date" || source == "time") {
        //  Date time sources
        auto dateSource = DateSource::Create();
        auto timeSource = TimeSource::Create(*mainLoop, dateSource);
        sources.Register("date", dateSource);
        sources.Register("time", timeSource);
        return;
    }
    if (source == "displays") {
        // Initialized by manager later..
        return;
    }
    if (source == "networks") {
        auto networkSource = NetworkSource::Create(source, *mainLoop, scriptContext);
        if (!networkSource) {
            spdlog::error("Failed to initialize network source");
            return;
        }
        sources.Register(source, networkSource);
        networkSource->Initialize();
        return;
    }
    if (source == "audio") {
        switch (config.audio.soundServer) {
            case SoundServer::PulseAudio: {
                auto audioSource = PulseAudioSource::Create(source, mainLoop, scriptContext);
                if (!audioSource) {
                    spdlog::error("Failed to initialize PulseAudio source");
                    return;
                }
                sources.Register(source, std::move(audioSource));
                return;
            }
            default:
                spdlog::error("Unknown sound server for audio");
                return;
        }
    }
    if (source == "power") {
        auto powerSource = PowerSource::Create(source, *mainLoop, scriptContext);
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
            sources.Register(source, registry.seat->keyboard);
            registry.seat->keyboard->SetScriptContext(source, scriptContext);
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
    // Initialize Lua context
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
    const auto config = scriptContext->Execute(configPath->c_str());
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
    // Initialize registry.
    // The registry initializes roots that contains elementary interfaces needed for the system
    // to work. The registry also maintains the list of active outputs (monitors).
    auto registry = Registry::Create(mainLoop, Outputs::Create(config));
    if (!registry) {
        spdlog::error("Failed to initialize registry");
        return -1;
    }
    // Initialize sources
    auto sources = Sources::Create();
    for (auto panelConfig : config->panels) {
        //  Check what sources are needed for the widgets in the panel
        for (const auto& widgetConfig : panelConfig.widgets) {
            for (const auto& source : widgetConfig.sources) {
                // Initialize source if not already done
                if (!sources->IsRegistered(source)) {
                    InitializeSource(source, *sources, mainLoop, scriptContext, *registry, *config);
                }
            }
        }
    }
    // Manager handles displays and redrawing
    auto manager =
        Manager::Create(registry, "displays", *mainLoop, std::move(sources), scriptContext);
    // Initialize compositor
    switch (config->displays.compositor) {
        case Compositor::Sway: {
            auto sway = SwayCompositor::Connect(*mainLoop, manager);
            if (!sway) {
                spdlog::error("Failed to connect to Sway");
                return -1;
            }
            // Leave the rest to main loop
            mainLoop->Run();
            break;
        }
        default:
            spdlog::error("Unsupported window manager");
            break;
    }
    return 0;
}
