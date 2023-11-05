#pragma once

#include <wayland-client-protocol.h>

#include <memory>
#include <vector>

#include "src/Roots.h"
#include "src/ScriptContext.h"
#include "src/Source.h"

class Keyboard : public Source {
   public:
    Keyboard(wl_keyboard* wlkeyboard) : m_wlkeyboard(wlkeyboard) {}
    static std::unique_ptr<Keyboard> Create(wl_seat* seat);
    void SetLayout(const char* layout);
    void SetScriptContext(std::shared_ptr<ScriptContext> scriptContext);

   private:
    wl_keyboard* m_wlkeyboard;
    KeyboardState m_sourceState;
    std::shared_ptr<ScriptContext> m_scriptContext;
};

class Seat {
   public:
    static std::unique_ptr<Seat> Create(const Roots& roots, wl_seat* seat);
    std::shared_ptr<Keyboard> keyboard;

   private:
    wl_seat* m_wlseat;
};
