#include "renderer/buffers.hpp"

#include <vulkan/vulkan.hpp>

#include "core/assert.hpp"
#include "renderer/command.hpp"
#include "renderer/context.hpp"

namespace mag
{
    // VulkanBuffer
    // -----------------------------------------------------------------------------------------------------------------
    VulkanBuffer::VulkanBuffer() = default;
    VulkanBuffer::~VulkanBuffer() = default;

    void VulkanBuffer::initialize(const u64 size_bytes, const vk::BufferUsageFlags usage,
                                  const VmaMemoryUsage memory_usage, const VmaAllocationCreateFlags memory_flags)
    {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size_bytes;
        buffer_create_info.usage = static_cast<VkBufferUsageFlags>(usage);

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = memory_usage;
        allocation_create_info.flags = memory_flags | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        buffer = new vk::Buffer();

        VK_CHECK(VK_CAST(vmaCreateBuffer(get_context().get_allocator(), &buffer_create_info, &allocation_create_info,
                                         reinterpret_cast<VkBuffer*>(buffer), &allocation, nullptr)));

        size = size_bytes;

        // Use persistent mapping
        this->map_memory();
    }

    void VulkanBuffer::shutdown()
    {
        this->unmap_memory();
        vmaDestroyBuffer(get_context().get_allocator(), *buffer, allocation);
        delete buffer;
        buffer = nullptr;
    }

    void* VulkanBuffer::map_memory()
    {
        VK_CHECK(VK_CAST(vmaMapMemory(get_context().get_allocator(), allocation, &mapped_region)));
        return mapped_region;
    }

    void VulkanBuffer::unmap_memory() { vmaUnmapMemory(get_context().get_allocator(), allocation); }

    void VulkanBuffer::copy(const void* data, const u64 size_bytes, const u64 offset)
    {
        ASSERT(offset + size_bytes <= size, "Size limit exceeded");
        memcpy(static_cast<c8*>(mapped_region) + offset, data, size_bytes);
    }

    const void* VulkanBuffer::get_handle() const { return buffer; }

    const void* VulkanBuffer::get_allocation() const { return allocation; }

    void* VulkanBuffer::get_data() const { return mapped_region; }

    u64 VulkanBuffer::get_size() const { return size; }

    u64 VulkanBuffer::get_device_address() const { return get_context().get_device().getBufferAddressKHR({*buffer}); }

    // GPUBuffer
    // -----------------------------------------------------------------------------------------------------------------
    GPUBuffer::GPUBuffer() = default;
    GPUBuffer::~GPUBuffer() = default;

    void GPUBuffer::initialize(const void* data, const u64 size_bytes, const vk::BufferUsageFlags usage)
    {
        auto& context = get_context();

        // Create staging buffer to send data to the gpu
        VulkanBuffer staging_buffer;
        staging_buffer.initialize(size_bytes, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        staging_buffer.copy(data, size_bytes);

        buffer.initialize(size_bytes, usage | vk::BufferUsageFlagBits::eTransferDst,
                          VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        // Copy data from the staging buffer to the gpu buffer
        context.submit_commands_immediate([&staging_buffer, size_bytes, this](CommandBuffer& cmd)
                                          { cmd.copy_buffer(staging_buffer, buffer, size_bytes, 0, 0); });

        staging_buffer.shutdown();
    }

    void GPUBuffer::shutdown() { buffer.shutdown(); }

    VulkanBuffer& GPUBuffer::get_buffer() { return buffer; }

    // VertexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    VertexBuffer::VertexBuffer(const void* vertices, const u64 size_bytes)
    {
        gpu_buffer.initialize(vertices, size_bytes, vk::BufferUsageFlagBits::eVertexBuffer);
    }

    VertexBuffer::~VertexBuffer() { gpu_buffer.shutdown(); }

    void VertexBuffer::resize(const void* vertices, const u64 size_bytes)
    {
        gpu_buffer.shutdown();
        gpu_buffer.initialize(vertices, size_bytes, vk::BufferUsageFlagBits::eVertexBuffer);
    }

    VulkanBuffer& VertexBuffer::get_buffer() { return gpu_buffer.get_buffer(); }

    // IndexBuffer
    // -----------------------------------------------------------------------------------------------------------------
    IndexBuffer::IndexBuffer(const void* indices, const u64 size_bytes)
    {
        gpu_buffer.initialize(indices, size_bytes, vk::BufferUsageFlagBits::eIndexBuffer);
    }

    IndexBuffer::~IndexBuffer() { gpu_buffer.shutdown(); }

    void IndexBuffer::resize(const void* indices, const u64 size_bytes)
    {
        gpu_buffer.shutdown();
        gpu_buffer.initialize(indices, size_bytes, vk::BufferUsageFlagBits::eIndexBuffer);
    }

    VulkanBuffer& IndexBuffer::get_buffer() { return gpu_buffer.get_buffer(); }
};  // namespace mag
