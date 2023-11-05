#pragma once

#include <wayland-client-protocol.h>

#include <memory>
#include <vector>

#include "src/Roots.h"

class Keyboard {
   public:
    Keyboard(wl_keyboard* wlkeyboard) : m_wlkeyboard(wlkeyboard) {}
    static std::unique_ptr<Keyboard> Create(wl_seat* seat);
    std::string layout;

   private:
    wl_keyboard* m_wlkeyboard;
};

class Seat {
   public:
    static std::unique_ptr<Seat> Create(const Roots& roots, wl_seat* seat);

   private:
    std::unique_ptr<Keyboard> keyboard;
    wl_seat* m_wlseat;
};
