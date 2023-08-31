#pragma once

#include <cairo/cairo.h>
#include <wayland-client-protocol.h>

#include <memory>

class Buffer {
   public:
    static std::unique_ptr<Buffer> Create(wl_buffer *buffer, void *address, int cx, int cy,
                                          const std::string &name);
    virtual ~Buffer();
    void OnRelease();
    wl_buffer *Lock() {
        m_inUse = true;
        return m_wlbuffer;
    }

    const std::string name;

    cairo_t *GetCairoCtx() { return m_cr; }
    void Clear(uint8_t v);
    bool InUse() { return m_inUse; }
    int Cx() { return m_cx; }

   private:
    Buffer(wl_buffer *buffer, void *address, int cx, int cy, size_t sizeInBytes,
           const std::string &name);

    wl_buffer *m_wlbuffer;
    bool m_inUse;
    const void *m_address;
    const int m_cx;
    const int m_cy;
    const size_t m_sizeInBytes;
    const cairo_surface_t *m_cr_surface;
    cairo_t *m_cr;
};
