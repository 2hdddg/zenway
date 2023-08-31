#pragma once

// Represents a memory map with one or more buffers in it
#include <memory>
#include <vector>

#include "Buffer.h"
#include "Roots.h"

class BufferPool {
   public:
    static std::unique_ptr<BufferPool> Create(const std::shared_ptr<Roots>, const int n,
                                              const int cx, const int cy);
    std::shared_ptr<Buffer> Get();

   private:
    using Buffers = std::vector<std::shared_ptr<Buffer>>;

    BufferPool(Buffers &&buffers) : m_buffers(buffers) {}
    Buffers m_buffers;
};
