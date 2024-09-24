#include "renderer/buffers.hpp"

#include "core/assert.hpp"
#include "renderer/context.hpp"

namespace mag
{
    // VulkanBuffer
    // -----------------------------------------------------------------------------------------------------------------
    void VulkanBuffer::initialize(const u64 size_bytes, const VkBufferUsageFlags usage,
                                  const VmaMemoryUsage memory_usage, const VmaAllocationCreateFlags memory_flags)
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

    void VulkanBuffer::shutdown() { vmaDestroyBuffer(get_context().get_allocator(), buffer, allocation); }

    void* VulkanBuffer::map_memory()
    {
        VK_CHECK(VK_CAST(vmaMapMemory(get_context().get_allocator(), allocation, &mapped_region)));
        return mapped_region;
    }

    void VulkanBuffer::unmap_memory() { vmaUnmapMemory(get_context().get_allocator(), allocation); }

    void VulkanBuffer::copy(const void* data, const u64 size_bytes, const u64 offset)
    {
        ASSERT(offset + size_bytes <= size, "Size limit exceeded");

        this->map_memory();
        memcpy(static_cast<char*>(mapped_region) + offset, data, size_bytes);
        this->unmap_memory();
    }

    u64 VulkanBuffer::get_device_address() const { return get_context().get_device().getBufferAddressKHR({buffer}); };

    // GPUBuffer
    // -----------------------------------------------------------------------------------------------------------------
    void GPUBuffer::initialize(const void* data, const u64 size_bytes, const VkBufferUsageFlags usage)
    {
        auto& context = get_context();

        // Create staging buffer to send data to the gpu
        VulkanBuffer staging_buffer;
        staging_buffer.initialize(size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        staging_buffer.copy(data, size_bytes);

        buffer.initialize(size_bytes, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        // Copy data from the staging buffer to the gpu buffer
        context.submit_commands_immediate([=, this](CommandBuffer cmd)
                                          { cmd.copy_buffer(staging_buffer, buffer, size_bytes, 0, 0); });

        staging_buffer.shutdown();
    }

    void GPUBuffer::shutdown() { buffer.shutdown(); }

    // VertexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    VertexBuffer::VertexBuffer(const void* vertices, const u64 size_bytes)
    {
        gpu_buffer.initialize(vertices, size_bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    VertexBuffer::~VertexBuffer() { gpu_buffer.shutdown(); }

    void VertexBuffer::resize(const void* vertices, const u64 size_bytes)
    {
        gpu_buffer.shutdown();
        gpu_buffer.initialize(vertices, size_bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    // IndexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    IndexBuffer::IndexBuffer(const void* indices, const u64 size_bytes)
    {
        gpu_buffer.initialize(indices, size_bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    IndexBuffer::~IndexBuffer() { gpu_buffer.shutdown(); }

    void IndexBuffer::resize(const void* indices, const u64 size_bytes)
    {
        gpu_buffer.shutdown();
        gpu_buffer.initialize(indices, size_bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
};  // namespace mag
