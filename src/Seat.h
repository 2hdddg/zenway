#pragma once

#include <wayland-client-protocol.h>

#include <memory>
#include <vector>

#include "src/MainLoop.h"
#include "src/ScriptContext.h"
#include "src/Sources.h"

using ClickHandler = std::function<void(wl_surface*, int, int)>;
using WheelHandler = std::function<void(wl_surface*, int, int, int)>;

class Keyboard : public Source {
   public:
    Keyboard(std::shared_ptr<MainLoop> mainloop, wl_keyboard* wlkeyboard)
        : Source(), m_mainloop(mainloop), m_wlkeyboard(wlkeyboard) {}
    virtual ~Keyboard() {
        wl_keyboard_destroy(m_wlkeyboard);
        m_wlkeyboard = nullptr;
    }
    static std::unique_ptr<Keyboard> Create(std::shared_ptr<MainLoop> mainloop, wl_seat* seat);
    void SetLayout(const char* layout);
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;

   private:
    std::shared_ptr<MainLoop> m_mainloop;
    wl_keyboard* m_wlkeyboard;
    KeyboardState m_sourceState;
};

class Pointer {
   public:
    static std::unique_ptr<Pointer> Create(wl_seat* seat);
    Pointer(wl_pointer* wlpointer) : m_wlpointer(wlpointer) {}
    virtual ~Pointer() {
        wl_pointer_destroy(m_wlpointer);
        m_wlpointer = nullptr;
    }
    void RegisterHandlers(ClickHandler clickHandler, WheelHandler wheelHandler) {
        m_clickHandler = clickHandler;
        m_wheelHandler = wheelHandler;
    }
    void Enter(wl_surface* surface) { m_current = surface; }
    void Leave() { m_current = nullptr; }
    void Track(wl_fixed_t x, wl_fixed_t y) {
        m_x = x;
        m_y = y;
    }
    void Click();
    void Wheel(int value);

   private:
    wl_pointer* m_wlpointer;
    wl_surface* m_current;
    wl_fixed_t m_x;
    wl_fixed_t m_y;
    ClickHandler m_clickHandler;
    WheelHandler m_wheelHandler;
};

class Seat {
   public:
    static std::unique_ptr<Seat> Create(std::shared_ptr<MainLoop> mainLoop, wl_seat* seat);
    Seat(wl_seat* wlseat) : m_wlseat(wlseat) {}
    virtual ~Seat() {
        wl_seat_destroy(m_wlseat);
        m_wlseat = nullptr;
    }

    void RegisterHandlers(ClickHandler clickHandler, WheelHandler wheelHandler) {
        // TODO: Log error
        if (!m_pointer) return;

        m_pointer->RegisterHandlers(clickHandler, wheelHandler);
    }

    std::shared_ptr<Keyboard> keyboard;

   private:
    std::unique_ptr<Pointer> m_pointer;
    wl_seat* m_wlseat;
};
