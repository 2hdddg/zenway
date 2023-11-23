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
    virtual ~Keyboard() {
        wl_keyboard_destroy(m_wlkeyboard);
        m_wlkeyboard = nullptr;
    }
    static std::unique_ptr<Keyboard> Create(wl_seat* seat);
    void SetLayout(const char* layout);
    void SetScriptContext(std::shared_ptr<ScriptContext> scriptContext);

   private:
    wl_keyboard* m_wlkeyboard;
    KeyboardState m_sourceState;
    std::shared_ptr<ScriptContext> m_scriptContext;
};

class Pointer {
   public:
    static std::unique_ptr<Pointer> Create(wl_seat* seat);
    Pointer(wl_pointer* wlpointer) : m_wlpointer(wlpointer) {}
    virtual ~Pointer() {
        wl_pointer_destroy(m_wlpointer);
        m_wlpointer = nullptr;
    }

   private:
    wl_pointer* m_wlpointer;
};

class Seat {
   public:
    static std::unique_ptr<Seat> Create(const Roots& roots, wl_seat* seat);
    Seat(wl_seat* wlseat) : m_wlseat(wlseat) {}
    virtual ~Seat() {
        wl_seat_destroy(m_wlseat);
        m_wlseat = nullptr;
    }

    std::shared_ptr<Keyboard> keyboard;

   private:
    std::unique_ptr<Pointer> m_pointer;
    wl_seat* m_wlseat;
};
