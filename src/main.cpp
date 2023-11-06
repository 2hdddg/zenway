#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <cstdlib>

#include "BufferPool.h"
#include "Configuration.h"
#include "DateTimeSources.h"
#include "MainLoop.h"
#include "Manager.h"
#include "Panel.h"
#include "PowerSource.h"
#include "PulseAudioSource.h"
#include "Registry.h"
#include "Sources.h"
#include "SwayCompositor.h"

int main(int argc, char* argv[]) {
    // Environment variable configurable logging
    spdlog::cfg::load_env_levels();
    auto scriptContext = std::shared_ptr<ScriptContext>(ScriptContext::Create());
    if (!scriptContext) {
        spdlog::error("Failed to create script context");
        return -1;
    }
    // Read configuration
    auto config = Configuration::Read(*scriptContext, argv[1]);
    if (!config) {
        spdlog::error("Failed to read configuration");
        return -1;
    }
    std::shared_ptr<MainLoop> mainLoop = MainLoop::Create();
    if (!mainLoop) {
        spdlog::error("Failed to initialize main loop");
        return -1;
    }
    // Initialize registry.
    // The registry initializes roots that contains elementary interfaces needed for the system
    // to work. The registry also maintains the list of active outputs (monitors).
    auto registry = Registry::Create(*mainLoop);
    if (!registry) {
        spdlog::error("Failed to initialize registry");
        return -1;
    }
    auto roots = registry->roots;
    // Create buffer pool
    std::shared_ptr<BufferPool> bufferPool =
        BufferPool::Create(roots, config->numBuffers, config->bufferWidth, config->bufferHeight);
    if (!bufferPool) {
        spdlog::error("Failed to initialize buffer pool");
        return -1;
    }
    // Initialize sources
    auto sources = Sources::Create(scriptContext);
    // Audio source
    auto audioSource = PulseAudioSource::Create(mainLoop, scriptContext);
    if (!audioSource) {
        spdlog::error("Failed to initialize PulseAudio source");
        return -1;
    }
    sources->Register("audio", std::move(audioSource));
    // Battery source
    auto powerSource = PowerSource::Create(*mainLoop, scriptContext);
    if (!powerSource) {
        spdlog::error("Failed to initialize battery source");
        return -1;
    }
    sources->Register("power", powerSource);
    // Initialize after registration
    powerSource->Initialize();
    //  Date time sources
    auto dateSource = DateSource::Create();
    auto timeSource = TimeSource::Create(*mainLoop, dateSource);
    sources->Register("date", dateSource);
    sources->Register("time", timeSource);
    // Seat sources
    if (registry->seat && registry->seat->keyboard) {
        sources->Register("keyboard", registry->seat->keyboard);
        registry->seat->keyboard->SetScriptContext(scriptContext);
    } else {
        spdlog::warn("No keyboard source");
    }

    // Panels
    std::vector<std::unique_ptr<Panel>> panels;
    auto panel = Panel::Create(bufferPool, config->leftPanel);
    panels.push_back(std::move(panel));
    panel = Panel::Create(bufferPool, config->rightPanel);
    panels.push_back(std::move(panel));

    auto manager =
        Manager::Create(*mainLoop, registry->outputs, std::move(sources), std::move(panels));

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
