#pragma once

#include "core/types.hpp"
#include "vk_mem_alloc.h"

// I dont want to wrap all these flags so we just fwd them for now :)
namespace vk
{
    template <typename BitType>
    class Flags;

    enum class BufferUsageFlagBits : unsigned int;
    using BufferUsageFlags = Flags<BufferUsageFlagBits>;

    class Buffer;
};  // namespace vk

namespace mag
{
    class Context;
    class CommandBuffer;

    class VulkanBuffer
    {
        public:
            VulkanBuffer();
            ~VulkanBuffer();

            void initialize(const u64 size_bytes, const vk::BufferUsageFlags usage, const VmaMemoryUsage memory_usage,
                            const VmaAllocationCreateFlags memory_flags);
            void shutdown();

            void* map_memory();
            void unmap_memory();
            void copy(const void* data, const u64 size_bytes, const u64 offset = 0);

            const void* get_handle() const;
            const void* get_allocation() const;
            void* get_data() const;

            u64 get_size() const;
            u64 get_device_address() const;

        private:
            vk::Buffer* buffer = nullptr;
            VmaAllocation allocation = {};
            void* mapped_region = {};
            u64 size = {};
    };

    class GPUBuffer
    {
        public:
            GPUBuffer();
            ~GPUBuffer();

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
