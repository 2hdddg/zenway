#include "Registry.h"

#include <spdlog/spdlog.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1.h>

#include <ostream>

#include "ShellSurface.h"

// Current oldest supported setup is Ubuntu 22.04 -> that ships with
//  - Sway version 1.7 ->
//    - wlroots version 0.15 ->
//      - wayland version 1.20 ->
//        - wl_shm -> version 1
//      - zwlr_layer_shell version 4
//      - wl_seat version 5
//      - wl_output version 4
//      - wl_compositor version 4
void Registry::Register(struct wl_registry *registry, uint32_t name, const char *interface,
                        uint32_t version) {
    uint32_t wanted_version = 0;
    uint32_t build_version = 0;
    if (interface == std::string_view(zwlr_layer_shell_v1_interface.name)) {
        // Not in wayland-protocols or core wayland.
        wanted_version = 3;  // 4 possible but minor thing related to keyboard interactivity not
                             // of interest right now. Current protocol file is ver 3, no need
                             // to update until something is needed.
        build_version = zwlr_layer_shell_v1_interface.version;
        this->shell = (zwlr_layer_shell_v1 *)wl_registry_bind(
            registry, name, &zwlr_layer_shell_v1_interface, wanted_version);
    } else if (interface == std::string_view(wl_shm_interface.name)) {
        // Defined in core wayland. There is a version 2 at time of writing..
        wanted_version = 1;
        build_version = wl_shm_interface.version;
        this->m_shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, wanted_version);
    } else if (interface == std::string_view(wl_compositor_interface.name)) {
        wanted_version = 4;
        build_version = wl_compositor_interface.version;
        this->compositor = (wl_compositor *)wl_registry_bind(
            registry, name, &wl_compositor_interface, wanted_version);
    } else if (interface == std::string_view(wl_output_interface.name)) {
        wanted_version = 4;
        build_version = wl_output_interface.version;
        auto output =
            (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, wanted_version);
        m_outputs->Add(output);
    } else if (interface == std::string_view(wl_seat_interface.name)) {
        if (seat) {
            spdlog::warn("Registration of additional seat, ignoring");
            return;
        }
        wanted_version = 5;
        build_version = wl_seat_interface.version;
        auto wlseat =
            (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface, wanted_version);
        seat = Seat::Create(m_mainloop, wlseat);
    } else {
        spdlog::trace("Ignored global interface: {} version {}", interface, version);
        return;
    }
    spdlog::info("Bound to global interface {} version {}. Max version {}. Built with version {}.",
                 interface, wanted_version, version, build_version);
}

void Registry::Unregister(struct wl_registry *, uint32_t /*name*/) {
    // TODO: Outputs will be removed here
}

static void on_register(void *data, struct wl_registry *wlregistry, uint32_t name,
                        const char *interface, uint32_t version) {
    auto registry = (Registry *)(data);
    registry->Register(wlregistry, name, interface, version);
}

static void on_unregister(void *data, struct wl_registry *wlregistry, uint32_t name) {
    auto registry = (Registry *)(data);
    registry->Unregister(wlregistry, name);
}

static const wl_registry_listener listener = {.global = on_register,
                                              .global_remove = on_unregister};

std::shared_ptr<Registry> Registry::Create(std::shared_ptr<MainLoop> mainLoop,
                                           std::unique_ptr<Outputs> outputs) {
    // Connect
    auto display = wl_display_connect(std::getenv("WAYLAND_DISPLAY"));
    if (display == nullptr) {
        spdlog::error("Failed to connect to wayland");
        return nullptr;
    }
    auto wlregistry = wl_display_get_registry(display);
    auto registry =
        std::shared_ptr<Registry>(new Registry(mainLoop, std::move(outputs), display, wlregistry));
    // Will fill in roots and this instance with needed interfaces
    wl_registry_add_listener(wlregistry, &listener, registry.get());
    // Two roundtrips, first to trigger registration, second to process binding requests.
    if (wl_display_roundtrip(display) < 0 || wl_display_roundtrip(display) < 0) return nullptr;
    // Register in mainloop
    mainLoop->Register(wl_display_get_fd(display), "wayland", registry);
    // Initialize buffers in outputs
    if (!registry->m_outputs->InitializeBuffers(*registry->m_shm)) {
        spdlog::error("Failed to initialize output buffers");
        return nullptr;
    }
    return registry;
}

bool Registry::OnRead() {
    wl_display_prepare_read(display);
    wl_display_read_events(display);
    wl_display_dispatch_pending(display);
    wl_display_flush(display);
    return false;
}

void Registry::FlushAndDispatchCommands() const {
    spdlog::trace("Flushing and dispatching wayland commands");
    wl_display_roundtrip(display);
}
