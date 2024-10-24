#pragma once

#include <vulkan/vulkan_handles.hpp>

#include "core/types.hpp"
#include "vk_mem_alloc.h"

namespace mag
{
    class Context;
    class CommandBuffer;

    class VulkanBuffer
    {
        public:
            void initialize(const u64 size_bytes, const vk::BufferUsageFlags usage, const VmaMemoryUsage memory_usage,
                            const VmaAllocationCreateFlags memory_flags);
            void shutdown();

            void* map_memory();
            void unmap_memory();
            void copy(const void* data, const u64 size_bytes, const u64 offset = 0);

            const vk::Buffer& get_buffer() const;
            const VmaAllocation& get_allocation() const;
            void* get_data() const;
            u64 get_size() const;
            u64 get_device_address() const;

        private:
            vk::Buffer buffer = {};
            VmaAllocation allocation = {};
            void* mapped_region = {};
            u64 size = {};
    };

    class GPUBuffer
    {
        public:
            void initialize(const void* data, const u64 size_bytes, const vk::BufferUsageFlags usage);
            void shutdown();

            VulkanBuffer& get_buffer();

        private:
            VulkanBuffer buffer;
    };

    class VertexBuffer
    {
        public:
            VertexBuffer(const void* vertices, const u64 size_bytes);
            ~VertexBuffer();

            void resize(const void* vertices, const u64 size_bytes);

            VulkanBuffer& get_buffer();

        private:
            GPUBuffer gpu_buffer;
    };

    class IndexBuffer
    {
        public:
            IndexBuffer(const void* indices, const u64 size_bytes);
            ~IndexBuffer();

            void resize(const void* indices, const u64 size_bytes);

            VulkanBuffer& get_buffer();

        private:
            GPUBuffer gpu_buffer;
    };
};  // namespace mag
