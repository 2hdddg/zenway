#pragma once

#include <wlr-layer-shell-unstable-v1.h>

#include <memory>

struct Roots {
    // Do not copy these!
    zwlr_layer_shell_v1 *shell;
    wl_shm *shm;
    wl_compositor *compositor;
    wl_display *display;

    virtual ~Roots() {
        zwlr_layer_shell_v1_destroy(shell);
        wl_shm_destroy(shm);
        wl_compositor_destroy(compositor);
        // Should be last!
        wl_display_disconnect(display);
    }

    void FlushAndDispatchCommands() const;
};
