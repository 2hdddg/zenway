#pragma once

#include <wayland-client-protocol.h>

#include <memory>
#include <vector>

#include "src/MainLoop.h"
#include "src/Roots.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

using ClickHandler = std::function<void(wl_surface*, int, int)>;

class Keyboard : public Source {
   public:
    Keyboard(std::shared_ptr<MainLoop> mainloop, wl_keyboard* wlkeyboard)
        : m_mainloop(mainloop), m_wlkeyboard(wlkeyboard) {}
    virtual ~Keyboard() {
        wl_keyboard_destroy(m_wlkeyboard);
        m_wlkeyboard = nullptr;
    }
    static std::unique_ptr<Keyboard> Create(std::shared_ptr<MainLoop> mainloop, wl_seat* seat);
    void SetLayout(const char* layout);
    void SetScriptContext(std::shared_ptr<ScriptContext> scriptContext);

   private:
    std::shared_ptr<MainLoop> m_mainloop;
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
    void RegisterClickHandler(ClickHandler handler) { m_clickHandler = handler; }
    void Enter(wl_surface* surface) { m_current = surface; }
    void Leave() { m_current = nullptr; }
    void Track(wl_fixed_t x, wl_fixed_t y) {
        m_x = x;
        m_y = y;
    }
    void Click();

   private:
    wl_pointer* m_wlpointer;
    wl_surface* m_current;
    wl_fixed_t m_x;
    wl_fixed_t m_y;
    ClickHandler m_clickHandler;
};

class Seat {
   public:
    static std::unique_ptr<Seat> Create(const Roots& roots, std::shared_ptr<MainLoop> mainLoop,
                                        wl_seat* seat);
    Seat(wl_seat* wlseat) : m_wlseat(wlseat) {}
    virtual ~Seat() {
        wl_seat_destroy(m_wlseat);
        m_wlseat = nullptr;
    }

    void RegisterClickHandler(ClickHandler handler) {
        // TODO: Log error
        if (!m_pointer) return;

        m_pointer->RegisterClickHandler(handler);
    }

    std::shared_ptr<Keyboard> keyboard;

   private:
    std::unique_ptr<Pointer> m_pointer;
    wl_seat* m_wlseat;
};
