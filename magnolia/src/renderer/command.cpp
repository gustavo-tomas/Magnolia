#include "renderer/command.hpp"

#include <vulkan/vulkan.hpp>

#include "renderer/context.hpp"
#include "renderer/renderer_image.hpp"

namespace mag
{
    void CommandBuffer::initialize(const vk::CommandPool& pool, const vk::CommandBufferLevel level)
    {
        vk::CommandBufferAllocateInfo cmd_alloc_info(pool, level, 1);
        this->command_buffer = get_context().get_device().allocateCommandBuffers(cmd_alloc_info).front();
    }

    void CommandBuffer::begin()
    {
        vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        this->command_buffer.begin(begin_info);
    }

    void CommandBuffer::end() { this->command_buffer.end(); }

    void CommandBuffer::begin_rendering(const vk::RenderingInfo& rendering_info)
    {
        this->command_buffer.beginRendering(rendering_info);
    }

    void CommandBuffer::end_rendering() { this->command_buffer.endRendering(); }

    void CommandBuffer::draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                             const u32 first_instance)
    {
        this->command_buffer.draw(vertex_count, instance_count, first_vertex, first_instance);
    }

    void CommandBuffer::draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index,
                                     const i32 vertex_offset, const u32 first_instance)
    {
        this->command_buffer.drawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void CommandBuffer::bind_vertex_buffer(const VulkanBuffer& buffer, const u64 offset)
    {
        this->command_buffer.bindVertexBuffers(0, buffer.get_buffer(), offset);
    }

    void CommandBuffer::bind_index_buffer(const VulkanBuffer& buffer, const u64 offset)
    {
        this->command_buffer.bindIndexBuffer(buffer.get_buffer(), offset, vk::IndexType::eUint32);
    }

    void CommandBuffer::bind_descriptor_set(const vk::PipelineBindPoint bind_point, const vk::PipelineLayout layout,
                                            const u32 first_set, const vk::DescriptorSet descriptor_set)
    {
        this->command_buffer.bindDescriptorSets(bind_point, layout, first_set, descriptor_set, nullptr);
    }

    void CommandBuffer::copy_buffer(const VulkanBuffer& src, const VulkanBuffer& dst, const u64 size_bytes,
                                    const u64 src_offset, const u64 dst_offset)
    {
        vk::BufferCopy copy(src_offset, dst_offset, size_bytes);
        this->command_buffer.copyBuffer(src.get_buffer(), dst.get_buffer(), copy);
    }

    void CommandBuffer::copy_buffer_to_image(const VulkanBuffer& src, const RendererImage& image)
    {
        const vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, image.get_mip_levels(), 0, 1);
        const vk::ImageMemoryBarrier to_transfer_barrier(
            {}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
            {}, {}, image.get_image(), range);

        this->get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                                           vk::DependencyFlagBits::eByRegion, {}, {}, to_transfer_barrier);

        const vk::ImageSubresourceLayers image_subresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        const vk::BufferImageCopy copy_region(0, 0, 0, image_subresource, {}, image.get_extent());

        this->command_buffer.copyBufferToImage(src.get_buffer(), image.get_image(),
                                               vk::ImageLayout::eTransferDstOptimal, copy_region);

        vk::ImageMemoryBarrier to_readable_barrier = to_transfer_barrier;
        to_readable_barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        this->get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                           vk::PipelineStageFlagBits::eFragmentShader,
                                           vk::DependencyFlagBits::eByRegion, {}, {}, to_readable_barrier);
    }

    void CommandBuffer::copy_image_to_image(const vk::Image& src, const vk::Extent3D& src_extent, const vk::Image& dst,
                                            const vk::Extent3D& dst_extent)
    {
        const vk::ImageSubresourceLayers src_subresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        const vk::ImageSubresourceLayers dst_subresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);

        std::array<vk::Offset3D, 2> src_offsets = {};
        src_offsets[1].x = src_extent.width;
        src_offsets[1].y = src_extent.height;
        src_offsets[1].z = src_extent.depth;

        std::array<vk::Offset3D, 2> dst_offsets = {};
        dst_offsets[1].x = dst_extent.width;
        dst_offsets[1].y = dst_extent.height;
        dst_offsets[1].z = dst_extent.depth;

        const vk::ImageBlit blit_region(src_subresource, src_offsets, dst_subresource, dst_offsets);

        this->command_buffer.blitImage(src, vk::ImageLayout::eTransferSrcOptimal, dst,
                                       vk::ImageLayout::eTransferDstOptimal, blit_region, vk::Filter::eLinear);
    }

    void CommandBuffer::transfer_layout(const RendererImage& image, const vk::ImageLayout curr_layout,
                                        const vk::ImageLayout new_layout, const u32 base_mip_levels)
    {
        this->transfer_layout(image.get_image(), curr_layout, new_layout, base_mip_levels, image.get_mip_levels());
    }

    void CommandBuffer::transfer_layout(const vk::Image& image, const vk::ImageLayout curr_layout,
                                        const vk::ImageLayout new_layout, const u32 base_mip_levels,
                                        const u32 mip_levels)
    {
        vk::ImageMemoryBarrier image_barrier;
        image_barrier.setOldLayout(curr_layout)
            .setNewLayout(new_layout)
            .setImage(image)
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, base_mip_levels, mip_levels, 0, 1});

        this->get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                                           vk::PipelineStageFlagBits::eAllCommands, {}, nullptr, nullptr,
                                           image_barrier);
    }
};  // namespace mag
