#pragma once

#include "core/types.hpp"
#include "private/vulkan_fwd.hpp"

namespace mag
{
    class RendererImage;
    class VulkanBuffer;

    class CommandBuffer
    {
        public:
            CommandBuffer();
            ~CommandBuffer();

            void initialize(const vk::CommandPool& pool, const vk::CommandBufferLevel level);

            void begin();
            void end();
            void begin_rendering(const vk::RenderingInfo& rendering_info);
            void end_rendering();

            void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                      const u32 first_instance = 0);

            void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                              const i32 vertex_offset = 0, const u32 first_instance = 0);

            void bind_vertex_buffer(const VulkanBuffer& buffer, const u64 offset = 0);

            void bind_index_buffer(const VulkanBuffer& buffer, const u64 offset = 0);

            void bind_descriptor_set(const vk::PipelineBindPoint bind_point, const vk::PipelineLayout layout,
                                     const u32 first_set, const vk::DescriptorSet descriptor_set);

            void copy_buffer(const VulkanBuffer& src, const VulkanBuffer& dst, const u64 size_bytes,
                             const u64 src_offset, const u64 dst_offset);

            void copy_buffer_to_image(const VulkanBuffer& src, const RendererImage& image);

            void copy_image_to_image(const vk::Image& src, const vk::Extent3D& src_extent, const vk::Image& dst,
                                     const vk::Extent3D& dst_extent);

            void transfer_layout(const RendererImage& image, const vk::ImageLayout curr_layout,
                                 const vk::ImageLayout new_layout, const u32 base_mip_levels = 0);

            void transfer_layout(const vk::Image& image, const vk::ImageLayout curr_layout,
                                 const vk::ImageLayout new_layout, const u32 base_mip_levels = 0,
                                 const u32 mip_levels = 1);

            const vk::CommandBuffer& get_handle() const;

        private:
            vk::CommandBuffer* command_buffer = nullptr;
    };
};  // namespace mag
