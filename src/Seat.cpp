#include "Seat.h"

#include <sys/mman.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "spdlog/spdlog.h"

static void on_pointer_enter(void* data, struct wl_pointer* wl_pointer, uint32_t serial,
                             struct wl_surface* surface, wl_fixed_t surface_x,
                             wl_fixed_t surface_y) {
    spdlog::info("ptr enter {}: {},{}", (uint64_t)surface, surface_x, surface_y);
}
static void on_pointer_leave(void* data, struct wl_pointer* wl_pointer, uint32_t serial,
                             struct wl_surface* surface) {
    spdlog::info("ptr leave");
}
static void on_pointer_motion(void* data, struct wl_pointer* wl_pointer, uint32_t time,
                              wl_fixed_t surface_x, wl_fixed_t surface_y) {
    // spdlog::info("ptr motion {},{}", wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y));
}
static void on_pointer_button(void* data, struct wl_pointer* wl_pointer, uint32_t serial,
                              uint32_t time, uint32_t button, uint32_t state) {
    spdlog::info("ptr button");
}
void on_pointer_frame(void* data, struct wl_pointer* wl_pointer) {}

static const wl_pointer_listener pointer_listener = {
    .enter = on_pointer_enter,
    .leave = on_pointer_leave,
    .motion = on_pointer_motion,
    .button = on_pointer_button,
    .frame = on_pointer_frame,
};

std::unique_ptr<Pointer> Pointer::Create(wl_seat* seat) {
    auto wlpointer = wl_seat_get_pointer(seat);
    auto pointer = std::make_unique<Pointer>(wlpointer);
    wl_pointer_add_listener(wlpointer, &pointer_listener, pointer.get());
    return pointer;
}

static void on_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd,
                      uint32_t size) {
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        spdlog::error("Keyboard: Invalid keymap format");
        return;
    }
    spdlog::info("Keyboard: keymap received");
    auto ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    char* map_shm = (char*)mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    auto keymap = xkb_keymap_new_from_string(ctx, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
                                             XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size);
    close(fd);

    ((Keyboard*)data)->SetLayout(xkb_keymap_layout_get_name(keymap, 0));

    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
}

void on_repeat_info(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {}

static const wl_keyboard_listener keyboard_listener = {
    .keymap = on_keymap,
    .repeat_info = on_repeat_info,
};

std::unique_ptr<Keyboard> Keyboard::Create(wl_seat* seat) {
    auto wlkeyboard = wl_seat_get_keyboard(seat);
    auto keyboard = std::make_unique<Keyboard>(wlkeyboard);
    wl_keyboard_add_listener(wlkeyboard, &keyboard_listener, keyboard.get());
    return keyboard;
}

void Keyboard::SetLayout(const char* layout) {
    m_sourceDirtyFlag = m_sourceState.layout != layout;
    if (m_sourceDirtyFlag) {
        m_sourceState.layout = layout;
        if (m_scriptContext) m_scriptContext->Publish(m_sourceState);
    }
}
void Keyboard::SetScriptContext(std::shared_ptr<ScriptContext> scriptContext) {
    m_scriptContext = scriptContext;
    if (m_sourceDirtyFlag) m_scriptContext->Publish(m_sourceState);
}

std::unique_ptr<Seat> Seat::Create(const Roots& roots, wl_seat* wlseat) {
    auto keyboard = Keyboard::Create(wlseat);
    auto seat = std::make_unique<Seat>(wlseat);
    seat->keyboard = std::move(keyboard);
    seat->m_pointer = Pointer::Create(wlseat);
    return seat;
}
