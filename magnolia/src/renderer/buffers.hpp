#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "vk_mem_alloc.h"

namespace mag
{
    class Context;
    class CommandBuffer;

    class Buffer
    {
        public:
            void initialize(const u64 size_bytes, const VkBufferUsageFlags usage, const VmaMemoryUsage memory_usage,
                            const VmaAllocationCreateFlags memory_flags, const VmaAllocator allocator);
            void shutdown();

            void* map_memory();
            void unmap_memory();
            void copy(const void* data, const u64 size_bytes);

            const vk::Buffer& get_buffer() const { return buffer; };
            const VmaAllocation& get_allocation() const { return allocation; };

        private:
            vk::Buffer buffer = {};
            VkBuffer vk_buffer = {};
            VmaAllocation allocation = {};
            VmaAllocator allocator = {};
            void* mapped_region = {};
    };

    // @TODO: DRY
    class VertexBuffer
    {
        public:
            void initialize(const void* vertices, const u64 size_bytes, const VmaAllocator allocator);
            void shutdown();

            const Buffer& get_buffer() const { return gpu_buffer; };

        private:
            Buffer gpu_buffer;
    };

    class IndexBuffer
    {
        public:
            void initialize(const void* indices, const u64 size_bytes, const VmaAllocator allocator);
            void shutdown();

            const Buffer& get_buffer() const { return gpu_buffer; };

        private:
            Buffer gpu_buffer;
    };
};  // namespace mag
