#include "Buffer.h"

#include <spdlog/spdlog.h>
#include <wayland-client-protocol.h>

static void on_release(void *data, struct wl_buffer *wl_buffer) { ((Buffer *)data)->OnRelease(); }

const struct wl_buffer_listener listener = {
    .release = on_release,
};

Buffer::Buffer(wl_buffer *buffer, void *address, int cx, int cy, size_t sizeInBytes,
               const std::string &name)
    : name(name),
      m_wlbuffer(buffer),
      m_address(address),
      m_cx(cx),
      m_cy(cy),
      m_sizeInBytes(sizeInBytes) {}

std::unique_ptr<Buffer> Buffer::Create(wl_buffer *wlbuffer, void *address, int cx, int cy,
                                       const std::string &name) {
    const int stride = cx * 4;
    const int size = stride * cy;
    auto buffer = std::unique_ptr<Buffer>(new Buffer(wlbuffer, address, cx, cy, size, name));
    buffer->m_cr_surface = cairo_image_surface_create_for_data((uint8_t *)address,
                                                               CAIRO_FORMAT_ARGB32, cx, cy, stride);
    buffer->m_cr = cairo_create(const_cast<cairo_surface_t *>(buffer->m_cr_surface));
    wl_buffer_add_listener(wlbuffer, &listener, buffer.get());
    return buffer;
}

Buffer::~Buffer() {
    cairo_surface_destroy(const_cast<cairo_surface_t *>(m_cr_surface));
    cairo_destroy(m_cr);
    wl_buffer_destroy(m_wlbuffer);
}

void Buffer::Clear(uint8_t v) { memset(const_cast<void *>(m_address), v, m_sizeInBytes); }

void Buffer::OnRelease() {
    spdlog::debug("Event wl_buffer::release {}", name);
    m_inUse = false;
}
