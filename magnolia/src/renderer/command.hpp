#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/buffers.hpp"

namespace mag
{
    struct Pass;

    class CommandBuffer
    {
        public:
            void initialize(const vk::CommandPool& pool, const vk::CommandBufferLevel level);

            void begin();
            void end();
            void begin_pass(const Pass& pass);
            void end_pass(const Pass& pass);

            void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                      const u32 first_instance);
            void bind_vertex_buffer(const Buffer& buffer, const u64 offset);

            void copy_buffer(const Buffer& src, const Buffer& dst, const u64 size_bytes, const u64 src_offset,
                             const u64 dst_offset);

            void copy_image_to_image(const vk::Image& src, const vk::Extent3D& src_extent, const vk::Image& dst,
                                     const vk::Extent3D& dst_extent);

            void transfer_layout(const vk::Image& image, const vk::ImageLayout curr_layout,
                                 const vk::ImageLayout new_layout);

            const vk::CommandBuffer& get_handle() const { return this->command_buffer; }

        private:
            vk::CommandBuffer command_buffer;
    };
};  // namespace mag
