#pragma once

#include <cairo/cairo.h>
#include <wayland-client-protocol.h>

#include <memory>
#include <vector>

// Represents a single buffer used for rendering
class Buffer {
   public:
    static std::unique_ptr<Buffer> Create(wl_buffer *buffer, void *address, int cx, int cy);
    virtual ~Buffer();
    void OnRelease();
    wl_buffer *Lock() {
        m_inUse = true;
        return m_wlbuffer;
    }

    cairo_t *GetCairoCtx() { return m_cr; }
    void Clear(uint8_t v);
    bool InUse() { return m_inUse; }

   private:
    Buffer(wl_buffer *buffer, void *address, int cx, int cy, size_t sizeInBytes);

    wl_buffer *m_wlbuffer;
    bool m_inUse;
    const void *m_address;
    const int m_cx;
    const int m_cy;
    const size_t m_sizeInBytes;
    const cairo_surface_t *m_cr_surface;
    cairo_t *m_cr;
};

// Represents a memory map with one or more buffers in it
class BufferPool {
   public:
    static std::unique_ptr<BufferPool> Create(wl_shm &shm, const int n, const int cx, const int cy);
    std::shared_ptr<Buffer> Get();

   private:
    using Buffers = std::vector<std::shared_ptr<Buffer>>;

    BufferPool(Buffers &&buffers) : m_buffers(buffers) {}
    Buffers m_buffers;
};
