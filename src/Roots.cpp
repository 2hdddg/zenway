#include "Roots.h"

#include "spdlog/spdlog.h"

void Roots::FlushAndDispatchCommands() const {
    spdlog::trace("Flushing and dispatching wayland commands");
    wl_display_roundtrip(display);
}
