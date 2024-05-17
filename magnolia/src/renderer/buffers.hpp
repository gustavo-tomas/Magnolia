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
                            const VmaAllocationCreateFlags memory_flags);
            void shutdown();

            void* map_memory();
            void unmap_memory();
            void copy(const void* data, const u64 size_bytes, const u64 offset = 0);

            const vk::Buffer& get_buffer() const { return buffer; };
            const VmaAllocation& get_allocation() const { return allocation; };
            void* get_data() const { return mapped_region; };
            u64 get_size() const { return size; };
            u64 get_device_address() const;

        private:
            vk::Buffer buffer = {};
            VkBuffer vk_buffer = {};
            VmaAllocation allocation = {};
            void* mapped_region = {};
            u64 size = {};
    };

    // @TODO: DRY
    class VertexBuffer
    {
        public:
            void initialize(const void* vertices, const u64 size_bytes);
            void shutdown();

            const Buffer& get_buffer() const { return gpu_buffer; };

        private:
            Buffer gpu_buffer;
    };

    class IndexBuffer
    {
        public:
            void initialize(const void* indices, const u64 size_bytes);
            void shutdown();

            const Buffer& get_buffer() const { return gpu_buffer; };

        private:
            Buffer gpu_buffer;
    };
};  // namespace mag
