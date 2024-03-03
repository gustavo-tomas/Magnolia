#include "renderer/buffers.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    // Buffer
    // -----------------------------------------------------------------------------------------------------------------
    void Buffer::initialize(const u64 size_bytes, const VkBufferUsageFlags usage, const VmaMemoryUsage memory_usage,
                            const VmaAllocationCreateFlags memory_flags)
    {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size_bytes;
        buffer_create_info.usage = usage;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = memory_usage;
        allocation_create_info.flags = memory_flags;

        VK_CHECK(VK_CAST(vmaCreateBuffer(get_context().get_allocator(), &buffer_create_info, &allocation_create_info,
                                         &vk_buffer, &allocation, nullptr)));

        this->buffer = vk::Buffer(vk_buffer);
        this->size = size_bytes;
    }

    void Buffer::shutdown() { vmaDestroyBuffer(get_context().get_allocator(), buffer, allocation); }

    void* Buffer::map_memory()
    {
        VK_CHECK(VK_CAST(vmaMapMemory(get_context().get_allocator(), allocation, &mapped_region)));
        return mapped_region;
    }

    void Buffer::unmap_memory() { vmaUnmapMemory(get_context().get_allocator(), allocation); }

    void Buffer::copy(const void* data, const u64 size_bytes)
    {
        this->map_memory();
        memcpy(mapped_region, data, size_bytes);
        this->unmap_memory();
    }

    u64 Buffer::get_device_address() const { return get_context().get_device().getBufferAddressKHR({buffer}); };

    // VertexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    void VertexBuffer::initialize(const void* vertices, const u64 size_bytes)
    {
        auto& context = get_context();

        // Create staging buffer to send data to the gpu
        Buffer staging_buffer;
        staging_buffer.initialize(size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        staging_buffer.copy(vertices, size_bytes);

        gpu_buffer.initialize(size_bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0);

        // Copy data from the staging buffer to the gpu buffer
        context.submit_commands_immediate([=, this](CommandBuffer cmd)
                                          { cmd.copy_buffer(staging_buffer, gpu_buffer, size_bytes, 0, 0); });

        staging_buffer.shutdown();
    }

    void VertexBuffer::shutdown() { gpu_buffer.shutdown(); }

    // IndexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    void IndexBuffer::initialize(const void* indices, const u64 size_bytes)
    {
        auto& context = get_context();

        // Create staging buffer to send data to the gpu
        Buffer staging_buffer;
        staging_buffer.initialize(size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        staging_buffer.copy(indices, size_bytes);

        gpu_buffer.initialize(
            size_bytes,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0);

        // Copy data from the staging buffer to the gpu buffer
        context.submit_commands_immediate([=, this](CommandBuffer cmd)
                                          { cmd.copy_buffer(staging_buffer, gpu_buffer, size_bytes, 0, 0); });

        staging_buffer.shutdown();
    }

    void IndexBuffer::shutdown() { gpu_buffer.shutdown(); }
};  // namespace mag
