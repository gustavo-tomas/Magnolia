#include "renderer/buffers.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    // Buffer
    // -----------------------------------------------------------------------------------------------------------------
    void Buffer::create(const u64 size_bytes, const VkBufferUsageFlags usage, const VmaMemoryUsage memory_usage,
                        const VmaAllocationCreateFlags memory_flags, const VmaAllocator allocator)
    {
        this->allocator = allocator;

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size_bytes;
        buffer_create_info.usage = usage;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = memory_usage;
        allocation_create_info.flags = memory_flags;

        VK_CHECK(VK_CAST(vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &vk_buffer,
                                         &allocation, nullptr)));

        this->buffer = vk::Buffer(vk_buffer);
    }

    void Buffer::destroy() { vmaDestroyBuffer(allocator, buffer, allocation); }

    void* Buffer::map_memory()
    {
        VK_CHECK(VK_CAST(vmaMapMemory(allocator, allocation, &mapped_region)));
        return mapped_region;
    }

    void Buffer::unmap_memory() { vmaUnmapMemory(allocator, allocation); }

    void Buffer::copy(const void* data, const u64 size_bytes)
    {
        this->map_memory();
        memcpy(mapped_region, data, size_bytes);
        this->unmap_memory();
    }

    const vk::Buffer& Buffer::get_buffer() const { return buffer; }

    const VmaAllocation& Buffer::get_allocation() const { return allocation; }

    // VertexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    void VertexBuffer::create(const void* vertices, const u64 size_bytes, const VmaAllocator allocator)
    {
        buffer.create(size_bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0, allocator);

        staging_buffer.create(size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocator);

        staging_buffer.copy(vertices, size_bytes);
    }

    void VertexBuffer::destroy()
    {
        buffer.destroy();
        staging_buffer.destroy();
    }

    const Buffer& VertexBuffer::get_buffer() const { return buffer; }

    const Buffer& VertexBuffer::get_staging_buffer() const { return staging_buffer; }

    // IndexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    void IndexBuffer::create(const void* indices, const u64 size_bytes, const VmaAllocator allocator)
    {
        buffer.create(size_bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, 0, allocator);

        staging_buffer.create(size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                              VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocator);

        staging_buffer.copy(indices, size_bytes);
    }

    void IndexBuffer::destroy()
    {
        buffer.destroy();
        staging_buffer.destroy();
    }

    const Buffer& IndexBuffer::get_buffer() const { return buffer; }

    const Buffer& IndexBuffer::get_staging_buffer() const { return staging_buffer; }
};  // namespace mag
