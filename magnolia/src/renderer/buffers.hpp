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
            void create(const u64 size_bytes, const VkBufferUsageFlags usage, const VmaMemoryUsage memory_usage,
                        const VmaAllocationCreateFlags memory_flags, const VmaAllocator allocator);
            void destroy();

            void* map_memory();
            void unmap_memory();
            void copy(const void* data, const u64 size_bytes);

            const vk::Buffer& get_buffer() const;
            const VmaAllocation& get_allocation() const;

        private:
            vk::Buffer buffer = {};
            VkBuffer vk_buffer = {};
            VmaAllocation allocation = {};
            VmaAllocator allocator = {};
            void* mapped_region = {};
    };

    class VertexBuffer
    {
        public:
            void create(const void* vertices, const u64 size_bytes, const VmaAllocator allocator);
            void destroy();

            // !TODO: lol
            const Buffer& get_buffer() const;
            const Buffer& get_staging_buffer() const;

        private:
            Buffer buffer, staging_buffer;
    };

    class IndexBuffer
    {
        public:
            void create(const void* indices, const u64 size_bytes, const VmaAllocator allocator);
            void destroy();

            // !TODO: lol
            const Buffer& get_buffer() const;
            const Buffer& get_staging_buffer() const;

        private:
            Buffer buffer, staging_buffer;
    };
};  // namespace mag
