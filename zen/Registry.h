#pragma once

#include <wayland-client-protocol.h>
#include <wlr-layer-shell-unstable-v1.h>

#include <cstdint>
#include <list>
#include <memory>

#include "zen/MainLoop.h"
#include "zen/Output.h"
#include "zen/Seat.h"
#include "zen/ShellSurface.h"

class Registry : public IoHandler {
   public:
    static std::shared_ptr<Registry> Create(std::shared_ptr<MainLoop> mainloop,
                                            std::unique_ptr<Outputs> outputs);
    virtual ~Registry() {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
        zwlr_layer_shell_v1_destroy(shell);
        shell = nullptr;
        wl_shm_destroy(m_shm);
        m_shm = nullptr;
        wl_compositor_destroy(compositor);
        compositor = nullptr;
        // Should be last!
        wl_display_disconnect(display);
        display = nullptr;
    }

    void FlushAndDispatchCommands() const;

    void Register(struct wl_registry *registry, uint32_t name, const char *interface,
                  uint32_t version);
    void Unregister(struct wl_registry *registry, uint32_t name);

    virtual bool OnRead() override;

    Outputs &BorrowOutputs() { return *m_outputs; }

    // These are maintained by the registry
    std::shared_ptr<Seat> seat;
    // Do not copy these!
    zwlr_layer_shell_v1 *shell;
    wl_compositor *compositor;
    wl_display *display;

   private:
    Registry(std::shared_ptr<MainLoop> mainloop, std::unique_ptr<Outputs> outputs,
             wl_display *display, wl_registry *registry)
        : m_outputs(std::move(outputs)), m_mainloop(mainloop), m_registry(registry) {
        this->display = display;
    }

   private:
    std::unique_ptr<Outputs> m_outputs;
    std::shared_ptr<MainLoop> m_mainloop;  // Hmm, this is circular..
    wl_registry *m_registry;
    wl_shm *m_shm;
};
