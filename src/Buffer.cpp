#include "Buffer.h"

#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/mman.h>

#include <cstring>

static void on_release(void *data, struct wl_buffer *) { ((Buffer *)data)->OnRelease(); }

const struct wl_buffer_listener listener = {
    .release = on_release,
};

Buffer::Buffer(wl_buffer *buffer, void *address, int cx, int cy, size_t sizeInBytes)
    : m_wlbuffer(buffer),
      m_inUse(false),
      m_address(address),
      m_cx(cx),
      m_cy(cy),
      m_sizeInBytes(sizeInBytes),
      m_cr_surface(nullptr),
      m_cr(nullptr) {}

std::unique_ptr<Buffer> Buffer::Create(wl_buffer *wlbuffer, void *address, int cx, int cy) {
    const int stride = cx * 4;
    const int size = stride * cy;
    auto buffer = std::unique_ptr<Buffer>(new Buffer(wlbuffer, address, cx, cy, size));
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
    spdlog::trace("Event wl_buffer::release");
    m_inUse = false;
}

std::unique_ptr<BufferPool> BufferPool::Create(wl_shm &shm, const int n, const int cx,
                                               const int cy) {
    auto fname = "/zenbuffers";
    int fd = shm_open(fname, O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        spdlog::error("Failed to shm_open double buffer: {}", strerror(errno));
        return nullptr;
    }
    const size_t stride = cx * 4;
    const size_t size = cy * stride;
    const size_t total_size = size * n;
    int ret = ftruncate(fd, total_size);
    if (ret == -1) {
        spdlog::error("Failed to set initial size of mem fd: ");
        return nullptr;
    }
    auto address = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) {
        spdlog::error("Failed to mmap initial fd: ");
        return nullptr;
    }
    auto pool = wl_shm_create_pool(&shm, fd, total_size);
    Buffers buffers(n);
    for (int i = 0; i < n; i++) {
        auto buffer = Buffer::Create(
            wl_shm_pool_create_buffer(pool, size * i, cx, cy, stride, WL_SHM_FORMAT_ARGB8888),
            ((uint8_t *)address) + (size * i), cx, cy);
        if (!buffer) {
            return nullptr;
        }
        buffers[i] = std::move(buffer);
    }
    return std::unique_ptr<BufferPool>(new BufferPool(std::move(buffers)));
}

// As long as only one thread is running this is ok
std::shared_ptr<Buffer> BufferPool::Get() {
    for (auto &buffer : m_buffers) {
        if (!buffer->InUse()) {
            return buffer;
        }
    }
    return nullptr;
}
