#include "BufferPool.h"

#include <errno.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

#include <cstdio>
#include <memory>

#include "Buffer.h"

std::unique_ptr<BufferPool> BufferPool::Create(const std::shared_ptr<Roots> roots, const int n,
                                               const int cx, const int cy) {
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
    auto pool = wl_shm_create_pool(roots->shm, fd, total_size);
    Buffers buffers(n);
    for (int i = 0; i < n; i++) {
        auto buffer = Buffer::Create(
            wl_shm_pool_create_buffer(pool, size * i, cx, cy, stride, WL_SHM_FORMAT_ARGB8888),
            ((uint8_t*)address) + (size * i), cx, cy, "zen.x");
        if (!buffer) {
            return nullptr;
        }
        buffers[i] = std::move(buffer);
    }
    return std::unique_ptr<BufferPool>(new BufferPool(std::move(buffers)));
}

// As long as only one thread is running this is ok
std::shared_ptr<Buffer> BufferPool::Get() {
    for (auto& buffer : m_buffers) {
        if (!buffer->InUse()) {
            return buffer;
        }
    }
    return nullptr;
}
