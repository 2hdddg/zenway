#include "zen/Seat.h"

#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "spdlog/spdlog.h"

static void on_pointer_enter(void* data, struct wl_pointer*, uint32_t /*serial*/,
                             struct wl_surface* surface, wl_fixed_t surface_x,
                             wl_fixed_t surface_y) {
    ((Pointer*)data)->Enter(surface);
    ((Pointer*)data)->Track(surface_x, surface_y);
    spdlog::trace("Pointer enter {}: {},{}", (uint64_t)surface, surface_x, surface_y);
}

static void on_pointer_leave(void* data, struct wl_pointer*, uint32_t /*serial*/,
                             struct wl_surface*) {
    spdlog::info("Pointer leave");
    ((Pointer*)data)->Leave();
}

static void on_pointer_motion(void* data, struct wl_pointer*, uint32_t /*time*/,
                              wl_fixed_t surface_x, wl_fixed_t surface_y) {
    ((Pointer*)data)->Track(surface_x, surface_y);
}

static void on_pointer_button(void* data, struct wl_pointer*, uint32_t /*serial*/,
                              uint32_t /*time*/, uint32_t button, uint32_t state) {
    spdlog::trace("Pointer click button {} state {}", button, state);
    if (state == 0 /*release*/ && button == BTN_LEFT) {
        ((Pointer*)data)->Click();
        return;
    }
}

static void on_axis(void* data, struct wl_pointer*, uint32_t /*time*/, uint32_t axis,
                    wl_fixed_t value) {
    spdlog::trace("on_axis, axis: {}, value: {}", axis, value);
    if (axis == WL_POINTER_AXIS_SOURCE_WHEEL) {
        ((Pointer*)data)->Wheel(value);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static const wl_pointer_listener pointer_listener = {
    .enter = on_pointer_enter,
    .leave = on_pointer_leave,
    .motion = on_pointer_motion,
    .button = on_pointer_button,
    .axis = on_axis,
};
#pragma GCC diagnostic pop

std::unique_ptr<Pointer> Pointer::Create(wl_seat* seat) {
    auto wlpointer = wl_seat_get_pointer(seat);
    auto pointer = std::make_unique<Pointer>(wlpointer);
    wl_pointer_add_listener(wlpointer, &pointer_listener, pointer.get());
    return pointer;
}

void Pointer::Click() {
    if (!m_current) return;
    if (!m_clickHandler) return;
    m_clickHandler(m_current, wl_fixed_to_int(m_x), wl_fixed_to_int(m_y));
}

void Pointer::Wheel(wl_fixed_t value) {
    if (!m_current) return;
    if (!m_wheelHandler) return;
    m_wheelHandler(m_current, wl_fixed_to_int(m_x), wl_fixed_to_int(m_y), wl_fixed_to_int(value));
}

static void on_keymap(void* data, struct wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        spdlog::error("Keyboard: Invalid keymap format");
        return;
    }
    auto ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    char* map_shm = (char*)mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    auto keymap = xkb_keymap_new_from_string(ctx, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
                                             XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size);
    close(fd);

    auto layout = xkb_keymap_layout_get_name(keymap, 0);
    ((Keyboard*)data)->SetLayout(layout);
    spdlog::info("Keyboard layout received: {}", layout);

    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
}

static void on_enter(void*, struct wl_keyboard*, uint32_t, struct wl_surface*, struct wl_array*) {}

static void on_leave(void*, struct wl_keyboard*, uint32_t, struct wl_surface*) {}
static void on_key(void*, struct wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t) {}
static void on_modifiers(void*, struct wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t,
                         uint32_t) {}
static void on_repeat_info(void*, struct wl_keyboard*, int32_t, int32_t) {}

static const wl_keyboard_listener keyboard_listener = {
    .keymap = on_keymap,
    .enter = on_enter,
    .leave = on_leave,
    .key = on_key,
    .modifiers = on_modifiers,
    .repeat_info = on_repeat_info,
};

std::unique_ptr<Keyboard> Keyboard::Create(std::shared_ptr<MainLoop> mainloop, wl_seat* seat) {
    spdlog::info("Creating keyboard");
    auto wlkeyboard = wl_seat_get_keyboard(seat);
    auto keyboard = std::make_unique<Keyboard>(mainloop, wlkeyboard);
    wl_keyboard_add_listener(wlkeyboard, &keyboard_listener, keyboard.get());
    return keyboard;
}

void Keyboard::SetLayout(const char* layout) {
    if (m_sourceState.layout != layout) {
        m_drawn = m_published = false;
        m_sourceState.layout = layout;
        m_mainloop->Wakeup();
    }
}

void Keyboard::Publish(const std::string_view sourceName, ScriptContext& scriptContext) {
    if (m_published) return;
    scriptContext.Publish(sourceName, m_sourceState);
    m_published = true;
}

std::unique_ptr<Seat> Seat::Create(std::shared_ptr<MainLoop> mainLoop, wl_seat* wlseat) {
    auto keyboard = Keyboard::Create(mainLoop, wlseat);
    auto seat = std::make_unique<Seat>(wlseat);
    seat->keyboard = std::move(keyboard);
    seat->m_pointer = Pointer::Create(wlseat);
    return seat;
}
