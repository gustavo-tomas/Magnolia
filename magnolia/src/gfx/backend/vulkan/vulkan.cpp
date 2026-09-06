#include "../backend.hpp"
// This one comes first

#include <vulkan/vulkan.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "VkBootstrap.h"
#include "VkBootstrapDispatch.h"
#include "conversions.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/debug.hpp"
#include "magnolia/core/memory.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/math/functions.hpp"
#include "magnolia/platform/window.hpp"

// Use to trace VMA allocations
#if MAG_CONFIG_DEBUG_TRACE
    #define VMA_DEBUG_LOG_FORMAT(format, ...) \
        do                                    \
        {                                     \
            printf((format), __VA_ARGS__);    \
            printf("\n");                     \
        } while (false)

    #define VMA_DEBUG_LOG(str) VMA_DEBUG_LOG_FORMAT("%s", (str))
#endif

#define VMA_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

namespace mag::gfx
{
    static constexpr void vk_check(const VkResult result, const str& message)
    {
        MAG_ASSERT((result) == VK_SUCCESS, "Vk check failed: '{}'", message);
    }

    struct VulkanSemaphore
    {
            VkSemaphore semaphore = {};
    };

    struct VulkanFence
    {
            VkFence fence = {};
    };

    struct VulkanSampler
    {
            VkSampler sampler = {};
    };

    struct VulkanCommandPool
    {
            VkCommandPool pool = {};
    };

    struct VulkanCommandBuffer
    {
            CommandPoolHandle command_pool = 0;
            VkCommandBuffer command_buffer = {};
    };

    struct VulkanBuffer
    {
            VkBuffer buffer = {};
            VmaAllocation allocation = nullptr;
            void* mapped_region = nullptr;
            u64 size = 0;
            BufferUsage usage = BufferUsage::Index;
    };

    struct VulkanDescriptorSetLayout
    {
            VkDescriptorSetLayout descriptor_layout = {};
    };

    struct VulkanDescriptorPool
    {
            VkDescriptorPool descriptor_pool = {};
    };

    struct VulkanDescriptorSet
    {
            VkDescriptorSet descriptor_set = {};
            DescriptorPoolHandle parent_pool_handle = 0;
    };

    struct VulkanTexture
    {
            VkImage image = {};
            VkImageView image_view = {};
            VmaAllocation allocation = nullptr;
            math::uvec3 extent = {1, 1, 1};
            Format format = Format::B8G8R8A8_SRGB;
            TextureType type = TextureType::Texture2D;
            TextureViewType view_type = TextureViewType::Texture2D;
            TextureAspect aspect = TextureAspect::Color;
            TextureUsage usage = TextureUsage::ColorAttachment;
            TextureLayout layout = TextureLayout::Undefined;
            u32 mip_levels = 1;
            u32 array_layers = 1;
            SampleCount sample_count = SampleCount::e1;
    };

    struct VulkanSwapchain
    {
            vkb::Swapchain swapchain;
            PresentMode present_mode = PresentMode::Mailbox;
            u32 current_image_index = 0;
            std::vector<TextureHandle> swapchain_textures;
    };

    struct VulkanRenderingAttachment
    {
            VkRenderingAttachmentInfoKHR rendering_attachment_info = {};
    };

    // @TODO: review this implementation
    struct VulkanRenderPass
    {
            VkRenderingInfoKHR render_info = {};

            // Color attachments need to be contiguous, but depth attachments dont.
            std::vector<VkRenderingAttachmentInfo> color_attachments;
            RenderingAttachmentHandle depth_attachment = {};
    };

    struct VulkanQueue
    {
            VkQueue queue = {};
    };

    struct VulkanGraphicsPipeline
    {
            VkPipelineLayout pipeline_layout = {};
            VkPipeline pipeline = {};
    };

    struct State
    {
            vkb::DispatchTable disp;
            vkb::Device device;
            vkb::Instance instance;
            vkb::InstanceDispatchTable inst_disp;
            VmaAllocator allocator = {};
            VkSurfaceKHR surface = {};

            CommandBufferHandle immediate_command_buffer_handle = 0;
            CommandPoolHandle immediate_command_pool_handle = 0;
            QueueHandle immediate_queue_handle = 0;
            FenceHandle immediate_fence_handle = 0;

            VulkanSwapchain swapchain = {};

            SemaphoreHandle semaphore_handles = 0;
            std::unordered_map<SemaphoreHandle, VulkanSemaphore> semaphores;

            FenceHandle fence_handles = 0;
            std::unordered_map<FenceHandle, VulkanFence> fences;

            SamplerHandle sampler_handles = 0;
            std::unordered_map<SamplerHandle, VulkanSampler> samplers;

            CommandPoolHandle command_pool_handles = 0;
            std::unordered_map<CommandPoolHandle, VulkanCommandPool> command_pools;

            CommandBufferHandle command_buffer_handles = 0;
            std::unordered_map<CommandBufferHandle, VulkanCommandBuffer> command_buffers;

            BufferHandle buffer_handles = 0;
            std::unordered_map<BufferHandle, VulkanBuffer> buffers;

            DescriptorSetLayoutHandle descriptor_set_layout_handles = 0;
            std::unordered_map<DescriptorSetLayoutHandle, VulkanDescriptorSetLayout> descriptor_layouts;

            DescriptorSetHandle descriptor_set_handles = 0;
            std::unordered_map<DescriptorSetHandle, VulkanDescriptorSet> descriptor_sets;

            DescriptorPoolHandle descriptor_pool_handles = 0;
            std::unordered_map<DescriptorPoolHandle, VulkanDescriptorPool> descriptor_pools;

            TextureHandle texture_handles = 0;
            std::unordered_map<TextureHandle, VulkanTexture> textures;

            RenderingAttachmentHandle rendering_attachment_handles = 0;
            std::unordered_map<RenderingAttachmentHandle, VulkanRenderingAttachment> rendering_attachments;

            RenderPassHandle render_pass_handles = 0;
            std::unordered_map<RenderPassHandle, VulkanRenderPass> render_passes;

            QueueHandle queue_handles = 0;
            std::unordered_map<QueueHandle, VulkanQueue> queues;

            GraphicsPipelineHandle graphics_pipeline_handles = 0;
            std::unordered_map<GraphicsPipelineHandle, VulkanGraphicsPipeline> graphics_pipelines;
    };

    static State* state = nullptr;

    void submit_commands_immediate(const std::function<void(const CommandBufferHandle)>& function);

    CommandBufferHandle create_command_buffer(const ICommandBufferDesc& desc)
    {
        const VulkanCommandPool& command_pool = state->command_pools[desc.command_pool];

        const CommandBufferHandle handle = state->command_buffer_handles++;

        VulkanCommandBuffer& command_buffer = state->command_buffers[handle];
        command_buffer.command_pool = desc.command_pool;

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool.pool;
        alloc_info.level = mag_to_vk(desc.command_buffer_level);
        alloc_info.commandBufferCount = 1;

        vk_check(state->disp.allocateCommandBuffers(&alloc_info, &command_buffer.command_buffer),
                 "Failed to allocate command buffer");

        return handle;
    }

    void destroy_command_buffer(const CommandBufferHandle handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanCommandPool& command_pool = state->command_pools[command_buffer.command_pool];

        state->disp.freeCommandBuffers(command_pool.pool, 1, &command_buffer.command_buffer);
    }

    void begin_recording_command_buffer(const CommandBufferHandle handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vk_check(state->disp.beginCommandBuffer(command_buffer.command_buffer, &begin_info),
                 "Failed to begin command buffer recording");
    }

    void end_recording_command_buffer(const CommandBufferHandle handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        vk_check(state->disp.endCommandBuffer(command_buffer.command_buffer), "Failed to record command buffer");
    }

    void reset_command_buffer(const CommandBufferHandle handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        vk_check(state->disp.resetCommandBuffer(command_buffer.command_buffer, 0), "Failed to reset command buffer");
    }

    void set_viewport_command_buffer(const CommandBufferHandle handle, const math::vec2& extent,
                                     const math::vec2& offset, const f32 min_depth, const f32 max_depth)
    {
        VkViewport viewport = {};
        viewport.width = extent.x;
        viewport.height = extent.y;
        viewport.x = offset.x;
        viewport.y = offset.y;
        viewport.minDepth = min_depth;
        viewport.maxDepth = max_depth;

        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        state->disp.cmdSetViewport(command_buffer.command_buffer, 0, 1, &viewport);
    }

    void set_scissor_command_buffer(const CommandBufferHandle handle, const math::uvec2& extent,
                                    const math::ivec2& offset)
    {
        VkRect2D scissor = {};
        scissor.extent = mag_to_vk(extent);
        scissor.offset = mag_to_vk(offset);

        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        state->disp.cmdSetScissor(command_buffer.command_buffer, 0, 1, &scissor);
    }

    void begin_rendering_command_buffer(const CommandBufferHandle handle, const RenderPassHandle render_pass_handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanRenderPass& render_pass = state->render_passes[render_pass_handle];

        const VkRenderingInfo* rendering_info = &render_pass.render_info;

        state->disp.cmdBeginRendering(command_buffer.command_buffer, rendering_info);
    }

    void end_rendering_command_buffer(const CommandBufferHandle handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        state->disp.cmdEndRendering(command_buffer.command_buffer);
    }

    void bind_pipeline_command_buffer(const CommandBufferHandle handle, const GraphicsPipelineHandle pipeline_handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanGraphicsPipeline& pipeline = state->graphics_pipelines[pipeline_handle];

        state->disp.cmdBindPipeline(command_buffer.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    }

    void bind_descriptor_command_buffer(const CommandBufferHandle handle, const GraphicsPipelineHandle pipeline_handle,
                                        const DescriptorSetHandle descriptor_handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanDescriptorSet& descriptor_set = state->descriptor_sets[descriptor_handle];

        const VulkanGraphicsPipeline& pipeline = state->graphics_pipelines[pipeline_handle];

        state->disp.cmdBindDescriptorSets(command_buffer.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          pipeline.pipeline_layout, 0, 1, &descriptor_set.descriptor_set, 0, nullptr);
    }

    void bind_vertex_buffers_command_buffer(const CommandBufferHandle handle, const u32 first_binding,
                                            const u32 binding_count, const std::vector<BufferHandle>& buffers,
                                            const std::vector<u64>& offsets)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        std::vector<VkBuffer> vk_buffers(buffers.size());

        for (u64 i = 0; i < buffers.size(); i++)
        {
            const VulkanBuffer& buffer = state->buffers[buffers[i]];
            vk_buffers[i] = buffer.buffer;
        }

        state->disp.cmdBindVertexBuffers(command_buffer.command_buffer, first_binding, binding_count, vk_buffers.data(),
                                         offsets.data());
    }

    void bind_index_buffer_command_buffer(const CommandBufferHandle handle, const BufferHandle buffer_handle,
                                          const u64 offset)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanBuffer& buffer = state->buffers[buffer_handle];

        state->disp.cmdBindIndexBuffer(command_buffer.command_buffer, buffer.buffer, offset, VK_INDEX_TYPE_UINT32);
    }

    void draw_command_buffer(const CommandBufferHandle handle, const u32 vertex_count, const u32 instance_count,
                             const u32 first_vertex, const u32 first_instance)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        state->disp.cmdDraw(command_buffer.command_buffer, vertex_count, instance_count, first_vertex, first_instance);
    }

    void draw_indexed_command_buffer(const CommandBufferHandle handle, const u32 index_count, const u32 instance_count,
                                     const u32 first_index, const i32 vertex_offset, const u32 first_instance)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        state->disp.cmdDrawIndexed(command_buffer.command_buffer, index_count, instance_count, first_index,
                                   vertex_offset, first_instance);
    }

    void draw_indexed_indirect_command_buffer(const CommandBufferHandle handle, const BufferHandle buffer_handle,
                                              const u64 offset, const u32 draw_count, const u32 stride)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanBuffer& buffer = state->buffers[buffer_handle];

        state->disp.cmdDrawIndexedIndirect(command_buffer.command_buffer, buffer.buffer, offset, draw_count, stride);
    }

    void pipeline_barrier_command_buffer(const CommandBufferHandle handle, const TextureHandle texture_handle,
                                         const TextureLayout new_layout, const AccessMask src_access_mask,
                                         const AccessMask dst_access_mask, const PipelineStage src_stage_mask,
                                         const PipelineStage dst_stage_mask)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        VulkanTexture& texture = state->textures[texture_handle];

        VkImageMemoryBarrier image_memory_barrier = {};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.srcAccessMask = mag_to_vk(src_access_mask);
        image_memory_barrier.dstAccessMask = mag_to_vk(dst_access_mask);
        image_memory_barrier.oldLayout = mag_to_vk(texture.layout);
        image_memory_barrier.newLayout = mag_to_vk(new_layout);
        image_memory_barrier.image = texture.image;
        image_memory_barrier.subresourceRange = {
            .aspectMask = mag_to_vk(texture.aspect),
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        state->disp.cmdPipelineBarrier(command_buffer.command_buffer, mag_to_vk(src_stage_mask),
                                       mag_to_vk(dst_stage_mask), VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr,
                                       1, &image_memory_barrier);

        texture.layout = new_layout;
    }

    void blit_texture_command_buffer(const CommandBufferHandle handle, const TextureHandle src_texture_handle,
                                     const TextureHandle dst_texture_handle, const Filter filter)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];

        const VulkanTexture& src_texture = state->textures[src_texture_handle];
        const VulkanTexture& dst_texture = state->textures[dst_texture_handle];

        VkImageBlit image_blit = {};

        // Src
        const math::uvec3& src_extent = src_texture.extent;

        image_blit.srcOffsets[1].x = static_cast<i32>(src_extent.x);
        image_blit.srcOffsets[1].y = static_cast<i32>(src_extent.y);
        image_blit.srcOffsets[1].z = static_cast<i32>(src_extent.z);

        image_blit.srcSubresource.layerCount = src_texture.array_layers;
        image_blit.srcSubresource.aspectMask = mag_to_vk(src_texture.aspect);
        image_blit.srcSubresource.baseArrayLayer = 0;
        image_blit.srcSubresource.mipLevel = 0;

        // Dst
        const math::uvec3& dst_extent = dst_texture.extent;

        image_blit.dstOffsets[1].x = static_cast<i32>(dst_extent.x);
        image_blit.dstOffsets[1].y = static_cast<i32>(dst_extent.y);
        image_blit.dstOffsets[1].z = static_cast<i32>(dst_extent.z);

        image_blit.dstSubresource.layerCount = dst_texture.array_layers;
        image_blit.dstSubresource.aspectMask = mag_to_vk(dst_texture.aspect);
        image_blit.dstSubresource.baseArrayLayer = 0;
        image_blit.dstSubresource.mipLevel = 0;

        state->disp.cmdBlitImage(command_buffer.command_buffer, src_texture.image, mag_to_vk(src_texture.layout),
                                 dst_texture.image, mag_to_vk(dst_texture.layout), 1, &image_blit, mag_to_vk(filter));
    }

    void copy_texture_command_buffer(const CommandBufferHandle handle, const TextureHandle src_texture_handle,
                                     const TextureHandle dst_texture_handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];
        const VulkanTexture& src_texture = state->textures[src_texture_handle];
        const VulkanTexture& dst_texture = state->textures[dst_texture_handle];

        const math::uvec3& extent = math::min(src_texture.extent, dst_texture.extent);

        VkImageCopy image_copy = {};
        image_copy.extent = mag_to_vk(extent);

        image_copy.srcSubresource.layerCount = src_texture.array_layers;
        image_copy.srcSubresource.aspectMask = mag_to_vk(src_texture.aspect);
        image_copy.srcOffset = {};

        image_copy.dstSubresource.layerCount = dst_texture.array_layers;
        image_copy.dstSubresource.aspectMask = mag_to_vk(dst_texture.aspect);
        image_copy.dstOffset = {};

        state->disp.cmdCopyImage(command_buffer.command_buffer, src_texture.image, mag_to_vk(src_texture.layout),
                                 dst_texture.image, mag_to_vk(dst_texture.layout), 1, &image_copy);
    }

    void copy_buffer_to_texture_command_buffer(const CommandBufferHandle handle, const BufferHandle buffer_handle,
                                               const TextureHandle texture_handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[handle];
        const VulkanBuffer& buffer = state->buffers[buffer_handle];
        const VulkanTexture& texture = state->textures[texture_handle];

        VkBufferImageCopy buffer_image_copy = {};
        buffer_image_copy.bufferImageHeight = texture.extent.y;
        buffer_image_copy.bufferRowLength = texture.extent.x;
        buffer_image_copy.imageExtent = mag_to_vk(texture.extent);
        buffer_image_copy.imageSubresource.aspectMask = mag_to_vk(texture.aspect);
        buffer_image_copy.imageSubresource.baseArrayLayer = 0;
        buffer_image_copy.imageSubresource.layerCount = texture.array_layers;
        buffer_image_copy.imageSubresource.mipLevel = 0;

        state->disp.cmdCopyBufferToImage(command_buffer.command_buffer, buffer.buffer, texture.image,
                                         mag_to_vk(texture.layout), 1, &buffer_image_copy);
    }

    FenceHandle create_fence(const IFenceDesc& desc)
    {
        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        if (desc.signaled)
        {
            fence_info.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
        }

        const FenceHandle handle = state->fence_handles++;

        VkFence* const fence = &state->fences[handle].fence;

        vk_check(state->disp.createFence(&fence_info, nullptr, fence), "Failed to create fence");

        return handle;
    }

    void destroy_fence(const FenceHandle handle)
    {
        const VkFence& fence = state->fences[handle].fence;

        state->disp.destroyFence(fence, nullptr);
    }

    void wait_fence(const FenceHandle handle, const u64 timeout)
    {
        const VkFence* fence = &state->fences[handle].fence;

        vk_check(state->disp.waitForFences(1, fence, VK_TRUE, timeout), "Failed to wait for fence");
    }

    void reset_fence(const FenceHandle handle)
    {
        const VkFence* fence = &state->fences[handle].fence;

        vk_check(state->disp.resetFences(1, fence), "Failed to reset fence");
    }

    QueueHandle create_queue(const IQueueDesc& desc)
    {
        const QueueHandle handle = state->queue_handles++;
        VulkanQueue& queue = state->queues[handle];

        const vkb::Result<VkQueue> queue_ret = state->device.get_queue(mag_to_vk(desc.queue_type));

        MAG_ASSERT(queue_ret, "{}", queue_ret.error().message());

        queue.queue = queue_ret.value();

        return handle;
    }

    void destroy_queue(const QueueHandle handle) { state->queues.erase(handle); }

    void submit_queue(const QueueHandle handle, const SemaphoreHandle wait_semaphore_handle,
                      const SemaphoreHandle signal_semaphore_handle, const FenceHandle fence_handle,
                      const CommandBufferHandle command_buffer_handle)
    {
        const VulkanCommandBuffer& command_buffer = state->command_buffers[command_buffer_handle];

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        const std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.pWaitDstStageMask = wait_stages.data();

        if (wait_semaphore_handle != Invalid_ID)
        {
            const VkSemaphore* semaphore = &state->semaphores[wait_semaphore_handle].semaphore;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = semaphore;
        }

        if (signal_semaphore_handle != Invalid_ID)
        {
            const VkSemaphore* semaphore = &state->semaphores[signal_semaphore_handle].semaphore;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = semaphore;
        }

        if (command_buffer_handle != Invalid_ID)
        {
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer.command_buffer;
        }

        reset_fence(fence_handle);

        const VulkanQueue& queue = state->queues[handle];
        const VulkanFence& fence = state->fences[fence_handle];

        vk_check(state->disp.queueSubmit(queue.queue, 1, &submit_info, fence.fence),
                 "Failed to submit draw command buffer");
    }

    Result present_queue(const QueueHandle handle, const SemaphoreHandle wait_semaphore_handle)
    {
        const VulkanSwapchain& swapchain = state->swapchain;

        const VulkanQueue& queue = state->queues[handle];

        const VulkanSemaphore& wait_semaphore = state->semaphores[wait_semaphore_handle];

        const u32 image_index = swapchain.current_image_index;

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &wait_semaphore.semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain.swapchain.swapchain;
        present_info.pImageIndices = &image_index;

        const VkResult result = state->disp.queuePresentKHR(queue.queue, &present_info);

        return vk_to_mag(result);
    }

    RenderingAttachmentHandle create_render_attachment(const IRenderingAttachmentDesc& desc)
    {
        VkClearValue clear_value = {};

        // Choose clear value based on the attachment type
        if (desc.type == RenderingAttachmentType::Color)
        {
            clear_value.color = mag_to_vk(desc.clear_color);
        }

        else
        {
            clear_value.depthStencil.depth = desc.clear_depth;
            clear_value.depthStencil.stencil = desc.clear_stencil;
        }

        const RenderingAttachmentHandle handle = state->rendering_attachment_handles++;

        VulkanRenderingAttachment& rendering_attachment = state->rendering_attachments[handle];

        VkRenderingAttachmentInfo& rendering_attachment_info = rendering_attachment.rendering_attachment_info;
        rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        rendering_attachment_info.imageView = state->textures[desc.texture].image_view;
        rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        rendering_attachment_info.clearValue = clear_value;

        return handle;
    }

    void destroy_rendering_attachment(const RenderingAttachmentHandle handle)
    {
        state->rendering_attachments.erase(handle);
    }

    math::vec4 get_clear_color_render_attachment(const RenderingAttachmentHandle handle)
    {
        const VkRenderingAttachmentInfo& rendering_attachment_info =
            state->rendering_attachments[handle].rendering_attachment_info;

        return vk_to_mag(rendering_attachment_info.clearValue.color);
    }

    f32 get_clear_depth_render_attachment(const RenderingAttachmentHandle handle)
    {
        const VkRenderingAttachmentInfo& rendering_attachment_info =
            state->rendering_attachments[handle].rendering_attachment_info;

        return rendering_attachment_info.clearValue.depthStencil.depth;
    }

    u32 get_clear_stencil_render_attachment(const RenderingAttachmentHandle handle)
    {
        const VkRenderingAttachmentInfo& rendering_attachment_info =
            state->rendering_attachments[handle].rendering_attachment_info;

        return rendering_attachment_info.clearValue.depthStencil.stencil;
    }

    RenderPassHandle create_render_pass(const IRenderPassDesc& desc)
    {
        const RenderPassHandle handle = state->render_pass_handles++;

        VulkanRenderPass& render_pass = state->render_passes[handle];

        const VkRect2D render_area = {
            .offset = mag_to_vk(desc.offset),
            .extent = mag_to_vk(desc.extent),
        };

        for (const RenderingAttachmentHandle color_attachment_handle : desc.color_attachments)
        {
            const VulkanRenderingAttachment& color_attachment = state->rendering_attachments[color_attachment_handle];
            render_pass.color_attachments.push_back(color_attachment.rendering_attachment_info);
        }

        render_pass.render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        render_pass.render_info.renderArea = render_area;
        render_pass.render_info.layerCount = 1;

        if (!render_pass.color_attachments.empty())
        {
            render_pass.render_info.colorAttachmentCount = render_pass.color_attachments.size();
            render_pass.render_info.pColorAttachments = render_pass.color_attachments.data();
        }

        const VulkanRenderingAttachment& depth_attachment = state->rendering_attachments[desc.depth_attachment];
        render_pass.depth_attachment = desc.depth_attachment;
        render_pass.render_info.pDepthAttachment = &depth_attachment.rendering_attachment_info;

        return handle;
    }  // namespace mag::gfx

    void destroy_render_pass(const RenderPassHandle handle) { state->render_passes.erase(handle); }

    math::ivec2 get_offset(const RenderPassHandle handle)
    {
        const VulkanRenderPass& render_pass = state->render_passes[handle];
        return vk_to_mag(render_pass.render_info.renderArea.offset);
    }

    math::uvec2 get_extent(const RenderPassHandle handle)
    {
        const VulkanRenderPass& render_pass = state->render_passes[handle];
        return vk_to_mag(render_pass.render_info.renderArea.extent);
    }

    void* map_buffer(BufferHandle handle);

    BufferHandle create_buffer(const IBufferDesc& desc)
    {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = desc.size_bytes;
        buffer_create_info.usage = mag_to_vk(desc.buffer_usage);

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = mag_to_vk(desc.memory_usage);
        allocation_create_info.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        const BufferHandle handle = state->buffer_handles++;

        VulkanBuffer& buffer = state->buffers[handle];
        buffer.size = desc.size_bytes;
        buffer.usage = desc.buffer_usage;

        vk_check(vmaCreateBuffer(state->allocator, &buffer_create_info, &allocation_create_info, &buffer.buffer,
                                 &buffer.allocation, nullptr),
                 "Failed to create buffer");

        // Use persistent mapping
        map_buffer(handle);

        return handle;
    }

    void destroy_buffer_shitty_name(const BufferHandle handle)
    {
        const VulkanBuffer& buffer = state->buffers[handle];

        vmaUnmapMemory(state->allocator, buffer.allocation);
        vmaDestroyBuffer(state->allocator, buffer.buffer, buffer.allocation);

        state->buffers.erase(handle);
    }

    void* map_buffer(const BufferHandle handle)
    {
        VulkanBuffer& buffer = state->buffers[handle];

        vk_check(vmaMapMemory(state->allocator, buffer.allocation, &buffer.mapped_region),
                 "Failed to map buffer memory");

        return buffer.mapped_region;
    }

    void unmap_buffer(const BufferHandle handle)
    {
        const VulkanBuffer& buffer = state->buffers[handle];
        vmaUnmapMemory(state->allocator, buffer.allocation);
    }

    void set_data_buffer(const BufferHandle handle, const void* const data, const u64 data_size, const u64 offset)
    {
        const VulkanBuffer& buffer = state->buffers[handle];

        MAG_ASSERT(offset + data_size <= buffer.size, "Size limit exceeded");
        if (offset + data_size > buffer.size)
        {
            return;
        }
        mem::copy(static_cast<c8*>(buffer.mapped_region) + offset, buffer.size, data, data_size, data_size);
    }

    u64 get_size_buffer(const BufferHandle handle)
    {
        const VulkanBuffer& buffer = state->buffers[handle];
        return buffer.size;
    }

    BufferUsage get_usage_buffer(const BufferHandle handle)
    {
        const VulkanBuffer& buffer = state->buffers[handle];
        return buffer.usage;
    }

    void* get_mapped_region_buffer(const BufferHandle handle)
    {
        const VulkanBuffer& buffer = state->buffers[handle];
        return buffer.mapped_region;
    }

    TextureHandle create_texture(const ITextureDesc& desc)
    {
        const TextureHandle handle = state->texture_handles++;

        VulkanTexture& texture = state->textures[handle];
        texture.array_layers = desc.array_layers;
        texture.aspect = desc.aspect;
        texture.extent = desc.extent;
        texture.mip_levels = desc.mip_levels;
        texture.sample_count = desc.sample_count;
        texture.format = desc.format;
        texture.type = desc.type;
        texture.view_type = desc.view_type;
        texture.usage = desc.usage;

        // Create image and image view
        const VkImageCreateInfo image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = mag_to_vk(desc.type),
            .format = mag_to_vk(desc.format),
            .extent = mag_to_vk(desc.extent),
            .mipLevels = desc.mip_levels,
            .arrayLayers = desc.array_layers,
            .samples = mag_to_vk(desc.sample_count),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = mag_to_vk(desc.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo vma_alloc_info = {};
        vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        vma_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        vk_check(vmaCreateImage(state->allocator, &image_create_info, &vma_alloc_info, &texture.image,
                                &texture.allocation, nullptr),
                 "Failed to create image");

        VkImageViewCreateInfo view_create_info = {};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image = texture.image;
        view_create_info.format = mag_to_vk(desc.format);
        view_create_info.viewType = mag_to_vk(desc.view_type);
        view_create_info.subresourceRange.aspectMask = mag_to_vk(desc.aspect);
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.layerCount = 1;
        view_create_info.subresourceRange.levelCount = 1;

        vk_check(state->disp.createImageView(&view_create_info, nullptr, &texture.image_view),
                 "Failed to create image view");

        return handle;
    }

    void destroy_texture(const TextureHandle handle)
    {
        VulkanTexture& texture = state->textures[handle];

        state->disp.destroyImageView(texture.image_view, nullptr);
        if (texture.allocation != nullptr)
        {
            vmaDestroyImage(state->allocator, texture.image, texture.allocation);
        }
    }

    void set_data_texture(const TextureHandle handle, const void* const data, const u64 size)
    {
        IBufferDesc staging_buffer_desc = {};
        staging_buffer_desc.buffer_usage = BufferUsage::TransferSrc;
        staging_buffer_desc.memory_usage = MemoryUsage::Auto;
        staging_buffer_desc.size_bytes = size;

        const BufferHandle staging_buffer_handle = create_buffer(staging_buffer_desc);

        set_data_buffer(staging_buffer_handle, data, size, 0);

        // @TODO: use KTX to generate mip maps: https://www.khronos.org/ktx/

        submit_commands_immediate([handle, staging_buffer_handle](const CommandBufferHandle cmd)
        {
            // Transition image layout to transfer dst
            pipeline_barrier_command_buffer(cmd, handle, TextureLayout::TransferDst, AccessMask::None,
                                            AccessMask::TransferWrite, PipelineStage::TopOfPipe,
                                            PipelineStage::Transfer);

            copy_buffer_to_texture_command_buffer(cmd, staging_buffer_handle, handle);

            // Transition image layout to shader read only
            pipeline_barrier_command_buffer(cmd, handle, TextureLayout::ShaderReadOnly, AccessMask::TransferWrite,
                                            AccessMask::ShaderRead, PipelineStage::Transfer,
                                            PipelineStage::FragmentShader);
        });
    }

    const math::uvec3& get_extent_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.extent;
    }

    Format get_format_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.format;
    }

    TextureLayout get_layout_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.layout;
    }

    TextureType get_type_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.type;
    }

    TextureViewType get_view_type_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.view_type;
    }

    TextureAspect get_aspect_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.aspect;
    }

    TextureUsage get_usage_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.usage;
    }

    SampleCount get_sample_count_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.sample_count;
    }

    u32 get_mip_levels_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.mip_levels;
    }

    u32 get_array_layers_texture(const TextureHandle handle)
    {
        const VulkanTexture& texture = state->textures[handle];
        return texture.array_layers;
    }

    void set_new_layout_texture(const TextureHandle handle, const VkImageLayout new_image_layout)
    {
        VulkanTexture& texture = state->textures[handle];
        texture.layout = vk_to_mag(new_image_layout);
    }

    void recreate_swapchain(const math::uvec2& extent)
    {
        VulkanSwapchain& swapchain = state->swapchain;

        vkb::SwapchainBuilder swapchain_builder{state->device};

        const vkb::Result<vkb::Swapchain> swap_ret =
            swapchain_builder.set_old_swapchain(swapchain.swapchain)
                .set_desired_extent(extent.x, extent.y)
                .set_desired_present_mode(mag_to_vk(swapchain.present_mode))
                .add_fallback_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
                .add_fallback_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
                .add_fallback_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                .build();

        MAG_ASSERT(swap_ret, "{0} {1}", swap_ret.error().message(), std::to_string(swap_ret.vk_result()));

        swapchain.swapchain_textures.clear();

        vkb::destroy_swapchain(swapchain.swapchain);

        swapchain.swapchain = swap_ret.value();

        const std::vector<VkImage>& swapchain_images = swapchain.swapchain.get_images().value();
        const std::vector<VkImageView>& swapchain_image_views = swapchain.swapchain.get_image_views().value();

        swapchain.swapchain_textures.resize(swapchain.swapchain.image_count);
        for (u32 i = 0; i < swapchain.swapchain.image_count; i++)
        {
            const TextureHandle texture_handle = state->texture_handles++;

            VulkanTexture& texture = state->textures[texture_handle];
            texture.extent = math::uvec3(vk_to_mag(swapchain.swapchain.extent), 1);
            texture.image = swapchain_images[i];
            texture.image_view = swapchain_image_views[i];
            texture.usage = TextureUsage::TransferDst;

            swapchain.swapchain_textures[i] = texture_handle;
        }
    }

    void create_swapchain(const ISwapchainDesc& desc)
    {
        state->swapchain.current_image_index = 0;
        state->swapchain.present_mode = desc.desired_present_mode;
        recreate_swapchain(desc.desired_extent);
    }

    void destroy_swapchain()
    {
        VulkanSwapchain& swapchain = state->swapchain;

        vkb::destroy_swapchain(swapchain.swapchain);
        swapchain.swapchain_textures.clear();
    }

    u32 get_current_image_index_swapchain()
    {
        const VulkanSwapchain& swapchain = state->swapchain;
        return swapchain.current_image_index;
    }

    u32 get_image_count_swapchain()
    {
        const VulkanSwapchain& swapchain = state->swapchain;
        return swapchain.swapchain.image_count;
    }

    math::uvec2 get_extent_swapchain()
    {
        const VulkanSwapchain& swapchain = state->swapchain;
        return vk_to_mag(swapchain.swapchain.extent);
    }

    Format get_format_swapchain()
    {
        const VulkanSwapchain& swapchain = state->swapchain;
        return vk_to_mag(swapchain.swapchain.image_format);
    }

    TextureHandle get_texture_swapchain(const u32 index)
    {
        const VulkanSwapchain& swapchain = state->swapchain;
        return swapchain.swapchain_textures[index];
    }

    Result acquire_next_image_swapchain(const SemaphoreHandle signal_semaphore_handle, const FenceHandle fence_handle)
    {
        VulkanSwapchain& swapchain = state->swapchain;

        VkSemaphore semaphore = state->semaphores[signal_semaphore_handle].semaphore;
        VkFence fence = nullptr;

        if (fence_handle != Invalid_ID)
        {
            fence = state->fences[fence_handle].fence;
        }

        const VkResult result = state->disp.acquireNextImageKHR(swapchain.swapchain, Timeout, semaphore, fence,
                                                                &swapchain.current_image_index);

        return vk_to_mag(result);
    }

    void resize_swapchain(const math::uvec2& extent) { recreate_swapchain(extent); }

    DescriptorSetHandle create_descriptor_set(const IDescriptorSetDesc& desc)
    {
        const DescriptorSetHandle handle = state->descriptor_set_handles++;

        VulkanDescriptorSet& descriptor_set = state->descriptor_sets[handle];
        descriptor_set.parent_pool_handle = desc.descriptor_pool;

        const VulkanDescriptorPool& descriptor_pool = state->descriptor_pools[desc.descriptor_pool];

        const VkDescriptorSetLayout& descriptor_layout =
            state->descriptor_layouts[desc.descriptor_layout].descriptor_layout;

        VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_info = {};
        variable_count_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        variable_count_info.descriptorSetCount = 1;
        variable_count_info.pDescriptorCounts = &desc.max_variable_descriptor_count;

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool.descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &descriptor_layout;
        alloc_info.pNext = &variable_count_info;

        vk_check(state->disp.allocateDescriptorSets(&alloc_info, &descriptor_set.descriptor_set),
                 "Failed to allocate descriptor sets");

        return handle;
    }

    void destroy_descriptor_set(const DescriptorSetHandle handle)
    {
        const VulkanDescriptorSet& descriptor_set = state->descriptor_sets[handle];
        const VulkanDescriptorPool& descriptor_pool = state->descriptor_pools[descriptor_set.parent_pool_handle];

        vk_check(state->disp.freeDescriptorSets(descriptor_pool.descriptor_pool, 1, &descriptor_set.descriptor_set),
                 "Failed to free descriptor set");
    }

    void update_descriptor_set(const DescriptorSetHandle handle, const BufferHandle buffer_handle, const u32 binding,
                               const u32 array_element, const DescriptorType descriptor_type, const u64 offset)
    {
        const VulkanDescriptorSet& descriptor_set = state->descriptor_sets[handle];
        const VulkanBuffer& buffer = state->buffers[buffer_handle];

        std::vector<VkWriteDescriptorSet> descriptor_writes;

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.buffer;
        buffer_info.offset = offset;
        buffer_info.range = buffer.size;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_set.descriptor_set;
        write.dstBinding = binding;
        write.dstArrayElement = array_element;
        write.descriptorType = mag_to_vk(descriptor_type);
        write.descriptorCount = 1;
        write.pBufferInfo = &buffer_info;

        descriptor_writes.push_back(write);

        state->disp.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
    }

    void update_descriptor_set(const DescriptorSetHandle handle, const TextureHandle texture_handle,
                               const SamplerHandle sampler_handle, const u32 binding, const u32 array_element,
                               const DescriptorType descriptor_type)
    {
        const VulkanDescriptorSet& descriptor_set = state->descriptor_sets[handle];
        const VulkanTexture& texture = state->textures[texture_handle];
        const VulkanSampler& sampler = state->samplers[sampler_handle];

        std::vector<VkWriteDescriptorSet> descriptor_writes;

        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = mag_to_vk(texture.layout);
        image_info.imageView = texture.image_view;
        image_info.sampler = sampler.sampler;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_set.descriptor_set;
        write.dstBinding = binding;
        write.dstArrayElement = array_element;
        write.descriptorType = mag_to_vk(descriptor_type);
        write.descriptorCount = 1;
        write.pImageInfo = &image_info;

        descriptor_writes.push_back(write);

        state->disp.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
    }

    DescriptorPoolHandle create_descriptor_pool(const IDescriptorPoolDesc& desc)
    {
        const DescriptorPoolHandle handle = state->descriptor_pool_handles++;

        VkDescriptorPool* const descriptor_pool = &state->descriptor_pools[handle].descriptor_pool;

        std::vector<VkDescriptorPoolSize> pool_sizes;
        for (const IDescriptorPoolSizeDesc& size_desc : desc.size_descs)
        {
            VkDescriptorPoolSize pool_size = {};
            pool_size.type = mag_to_vk(size_desc.type);
            pool_size.descriptorCount = size_desc.count;

            pool_sizes.push_back(pool_size);
        }

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = pool_sizes.size();
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = desc.max_sets;
        pool_info.flags =
            VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        vk_check(state->disp.createDescriptorPool(&pool_info, nullptr, descriptor_pool),
                 "Failed to create descriptor pool");

        return handle;
    }

    void destroy_descriptor_pool(const DescriptorPoolHandle handle)
    {
        const VkDescriptorPool& descriptor_pool = state->descriptor_pools[handle].descriptor_pool;

        state->disp.destroyDescriptorPool(descriptor_pool, nullptr);
    }

    DescriptorSetLayoutHandle create_descriptor_set_layout(const IDescriptorSetLayoutDesc& desc)
    {
        const DescriptorSetLayoutHandle handle = state->descriptor_set_layout_handles++;

        VkDescriptorSetLayout* const descriptor_layout = &state->descriptor_layouts[handle].descriptor_layout;

        std::vector<VkDescriptorSetLayoutBinding> bindings(desc.binding_descs.size());
        std::vector<VkDescriptorBindingFlags> flags(desc.binding_descs.size());

        for (u64 i = 0; i < desc.binding_descs.size(); i++)
        {
            const IDescriptorSetLayoutBindingDesc& binding_desc = desc.binding_descs[i];

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = binding_desc.binding;
            binding.descriptorType = mag_to_vk(binding_desc.descriptor_type);
            binding.descriptorCount = binding_desc.descriptor_count;
            binding.stageFlags = mag_to_vk(binding_desc.stages);

            VkDescriptorBindingFlags binding_flags =
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

            if (binding_desc.variable_descriptor_count)
            {
                binding_flags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
            }

            bindings[i] = binding;
            flags[i] = binding_flags;
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags = {};
        binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        binding_flags.bindingCount = bindings.size();
        binding_flags.pBindingFlags = flags.data();

        VkDescriptorSetLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = bindings.size();
        layout_info.pBindings = bindings.data();
        layout_info.pNext = &binding_flags;
        layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        vk_check(state->disp.createDescriptorSetLayout(&layout_info, nullptr, descriptor_layout),
                 "Failed to create descriptor set layout");

        return handle;
    }

    void destroy_descriptor_set_layout(const DescriptorSetLayoutHandle handle)
    {
        const VkDescriptorSetLayout& descriptor_layout = state->descriptor_layouts[handle].descriptor_layout;

        state->disp.destroyDescriptorSetLayout(descriptor_layout, nullptr);
    }

    CommandPoolHandle create_command_pool(const ICommandPoolDesc& desc)
    {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = state->device.get_queue_index(mag_to_vk(desc.queue_type)).value();

        const CommandPoolHandle handle = state->command_pool_handles++;

        VkCommandPool* const command_pool = &state->command_pools[handle].pool;

        vk_check(state->disp.createCommandPool(&pool_info, nullptr, command_pool), "Failed to create command pool");

        return handle;
    }

    void destroy_command_pool(const CommandPoolHandle handle)
    {
        const VkCommandPool& command_pool = state->command_pools[handle].pool;

        state->disp.destroyCommandPool(command_pool, nullptr);
    }

    void reset_command_pool(const CommandPoolHandle handle)
    {
        const VkCommandPool& command_pool = state->command_pools[handle].pool;

        vk_check(state->disp.resetCommandPool(command_pool, 0), "Failed to reset command pool");
    }

    SamplerHandle create_sampler(const ISamplerDesc& desc)
    {
        VkSamplerCreateInfo sampler_info = {};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.minFilter = mag_to_vk(desc.min_filter);
        sampler_info.magFilter = mag_to_vk(desc.mag_filter);
        sampler_info.mipmapMode = mag_to_vk(desc.mipmap_mode);
        sampler_info.addressModeU = mag_to_vk(desc.address_mode_u);
        sampler_info.addressModeV = mag_to_vk(desc.address_mode_v);
        sampler_info.addressModeW = mag_to_vk(desc.address_mode_w);
        sampler_info.minLod = desc.min_lod;
        sampler_info.maxLod = desc.max_lod;
        sampler_info.anisotropyEnable = static_cast<VkBool32>(desc.anisotropy_enable);
        sampler_info.maxAnisotropy = desc.max_anisotropy;

        const SamplerHandle handle = state->sampler_handles++;

        VkSampler* const sampler = &state->samplers[handle].sampler;

        vk_check(state->disp.createSampler(&sampler_info, nullptr, sampler), "Failed to create sampler");

        return handle;
    }

    void destroy_sampler(const SamplerHandle handle)
    {
        const VkSampler& sampler = state->samplers[handle].sampler;

        state->disp.destroySampler(sampler, nullptr);
    }

    SemaphoreHandle create_semaphore(const ISemaphoreDesc& desc)
    {
        (void)desc;

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        const SemaphoreHandle handle = state->semaphore_handles++;

        VkSemaphore* const semaphore = &state->semaphores[handle].semaphore;

        vk_check(state->disp.createSemaphore(&semaphore_info, nullptr, semaphore), "Failed to create semaphore");

        return handle;
    }

    void destroy_semaphore(const SemaphoreHandle handle)
    {
        const VkSemaphore& semaphore = state->semaphores[handle].semaphore;

        state->disp.destroySemaphore(semaphore, nullptr);
    }

    GraphicsPipelineHandle create_graphics_pipeline(const IGraphicsPipelineDesc& desc)
    {
        const GraphicsPipelineHandle handle = state->graphics_pipeline_handles++;
        VulkanGraphicsPipeline& pipeline = state->graphics_pipelines[handle];

        const u32 shader_module_count = desc.shader_modules.size();

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages(shader_module_count);
        std::vector<VkShaderModule> shader_modules(shader_module_count);

        for (u32 i = 0; i < shader_module_count; i++)
        {
            const IShaderModuleDesc& shader_module_desc = desc.shader_modules[i];

            VkShaderModuleCreateInfo shader_module_info = {};
            shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_info.codeSize = shader_module_desc.code.size();

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            shader_module_info.pCode = reinterpret_cast<const u32* const>(shader_module_desc.code.data());

            shader_modules[i] = {};

            vk_check(state->disp.createShaderModule(&shader_module_info, nullptr, &shader_modules[i]),
                     "Failed to create shader module");

            shader_stages[i] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = mag_to_vk_bits(shader_module_desc.stage),
                .module = shader_modules[i],
                .pName = "main",
                .pSpecializationInfo = nullptr,
            };
        }

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts(desc.descriptor_layouts.size());
        for (u64 i = 0; i < descriptor_set_layouts.size(); i++)
        {
            const DescriptorSetLayoutHandle descriptor_layout_handle = desc.descriptor_layouts[i];

            const VulkanDescriptorSetLayout& descriptor_set_layout =
                state->descriptor_layouts[descriptor_layout_handle];

            descriptor_set_layouts[i] = descriptor_set_layout.descriptor_layout;
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexAttributeDescriptionCount = desc.vertex_attribute_descs.size();
        vertex_input_info.vertexBindingDescriptionCount = desc.vertex_binding_descs.size();

        std::vector<VkVertexInputAttributeDescription> vertex_attribute_infos;
        std::vector<VkVertexInputBindingDescription> vertex_binding_infos;

        for (const IVertexAttributeDesc& vertex_attribute_desc : desc.vertex_attribute_descs)
        {
            VkVertexInputAttributeDescription vertex_attribute_info = {};

            vertex_attribute_info.format = mag_to_vk(vertex_attribute_desc.format);
            vertex_attribute_info.binding = vertex_attribute_desc.binding;
            vertex_attribute_info.location = vertex_attribute_desc.location;
            vertex_attribute_info.offset = vertex_attribute_desc.offset;

            vertex_attribute_infos.push_back(vertex_attribute_info);
        }

        for (const IVertexBindingDesc& vertex_binding_desc : desc.vertex_binding_descs)
        {
            VkVertexInputBindingDescription vertex_binding_info = {};

            vertex_binding_info.inputRate = mag_to_vk(vertex_binding_desc.input_rate);
            vertex_binding_info.binding = vertex_binding_desc.binding;
            vertex_binding_info.stride = vertex_binding_desc.stride;

            vertex_binding_infos.push_back(vertex_binding_info);
        }

        vertex_input_info.pVertexAttributeDescriptions = vertex_attribute_infos.data();
        vertex_input_info.pVertexBindingDescriptions = vertex_binding_infos.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = mag_to_vk(desc.primitive_topology);
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0F;
        viewport.y = 0.0F;
        viewport.width = static_cast<f32>(desc.extent.x);
        viewport.height = static_cast<f32>(desc.extent.y);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;

        VkRect2D scissor = {};
        scissor.offset = {.x = 0, .y = 0};
        scissor.extent = mag_to_vk(desc.extent);

        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0F;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        const VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0.0F,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment = {};
        color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = static_cast<VkBool32>(desc.color_blend.blend_enable);
        color_blend_attachment.colorBlendOp = mag_to_vk(desc.color_blend.color_blend_op);
        color_blend_attachment.srcColorBlendFactor = mag_to_vk(desc.color_blend.src_color_blend_factor);
        color_blend_attachment.dstColorBlendFactor = mag_to_vk(desc.color_blend.dst_color_blend_factor);
        color_blend_attachment.alphaBlendOp = mag_to_vk(desc.color_blend.alpha_blend_op);
        color_blend_attachment.srcAlphaBlendFactor = mag_to_vk(desc.color_blend.src_alpha_blend_factor);
        color_blend_attachment.dstAlphaBlendFactor = mag_to_vk(desc.color_blend.dst_alpha_blend_factor);

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 0;

        if (state->disp.createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline.pipeline_layout) != VK_SUCCESS)
        {
            MAG_ASSERT(false, "Failed to create pipeline layout");
        }

        std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_info = {};
        dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_info.dynamicStateCount = static_cast<u32>(dynamic_states.size());
        dynamic_info.pDynamicStates = dynamic_states.data();

        const VkFormat swapchain_format = mag_to_vk(desc.color_attachment_format);

        VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
        pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        pipeline_rendering_create_info.colorAttachmentCount = 1;
        pipeline_rendering_create_info.depthAttachmentFormat = mag_to_vk(desc.depth_attachment_format);
        pipeline_rendering_create_info.pColorAttachmentFormats = &swapchain_format;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.depthTestEnable = 1U;
        depth_stencil_create_info.depthWriteEnable = 1U;
        depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil_create_info.minDepthBounds = 0.0F;
        depth_stencil_create_info.maxDepthBounds = 1.0F;

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.pDepthStencilState = &depth_stencil_create_info;
        pipeline_info.stageCount = shader_module_count;
        pipeline_info.pStages = shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_info;
        pipeline_info.layout = pipeline.pipeline_layout;
        pipeline_info.renderPass = nullptr;
        pipeline_info.subpass = 0;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.pNext = &pipeline_rendering_create_info;

        if (state->disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline.pipeline) !=
            VK_SUCCESS)
        {
            MAG_ASSERT(false, "Failed to create pipeline");
        }

        for (u32 i = 0; i < shader_module_count; i++)
        {
            state->disp.destroyShaderModule(shader_modules[i], nullptr);
        }

        return handle;
    }

    void destroy_graphics_pipeline(const GraphicsPipelineHandle handle)
    {
        const VulkanGraphicsPipeline& pipeline = state->graphics_pipelines[handle];

        state->disp.destroyPipeline(pipeline.pipeline, nullptr);
        state->disp.destroyPipelineLayout(pipeline.pipeline_layout, nullptr);
    }

    void create_device()
    {
        state = new State();

        const u32 vulkan_major_version = 1;
        const u32 vulkan_minor_version = 3;
        const u32 vulkan_patch_version = 0;

        // Device
        // -------------------------------------------------------------------------------------------------
        vkb::InstanceBuilder instance_builder;
        const vkb::Result<vkb::Instance> instance_ret =
            instance_builder

#if MAG_CONFIG_DEBUG
                .set_debug_callback(
                    [](VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                       VkDebugUtilsMessageTypeFlagsEXT message_type,
                       const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) -> VkBool32
        {
            (void)message_type;
            (void)user_data;

            if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                LOG_WARNING("{0}\n", callback_data->pMessage);
            }

            else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                LOG_ERROR("{0}\n", callback_data->pMessage);
                DEBUG_BREAK();
            }

            else
            {
                LOG_INFO("{0}\n", callback_data->pMessage);
            }

            return VK_FALSE;
        })
                .request_validation_layers()

                .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT)
                .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
        // .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)
        // .add_validation_feature_enable(
        //     VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT)
#endif
                .require_api_version(vulkan_major_version, vulkan_minor_version, vulkan_patch_version)
                .build();

        MAG_ASSERT(instance_ret, "{}", instance_ret.error().message());

        state->instance = instance_ret.value();
        state->inst_disp = state->instance.make_table();

        window::create_surface(static_cast<void*>(&state->instance.instance), static_cast<void*>(&state->surface));

        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = {};
        dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
        dynamic_rendering_features.dynamicRendering = 1U;

        VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {};
        descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        descriptor_indexing_features.descriptorBindingPartiallyBound = 1U;
        descriptor_indexing_features.descriptorBindingVariableDescriptorCount = 1U;
        descriptor_indexing_features.descriptorBindingUniformBufferUpdateAfterBind = 1U;
        descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind = 1U;
        descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind = 1U;
        descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = 1U;
        descriptor_indexing_features.shaderStorageBufferArrayNonUniformIndexing = 1U;
        descriptor_indexing_features.shaderUniformBufferArrayNonUniformIndexing = 1U;
        descriptor_indexing_features.runtimeDescriptorArray = 1U;

        vkb::PhysicalDeviceSelector phys_device_selector(state->instance);
        const vkb::Result<vkb::PhysicalDevice> phys_device_ret =
            phys_device_selector.set_minimum_version(vulkan_major_version, vulkan_minor_version)
                .set_surface(state->surface)
                .add_required_extension_features(descriptor_indexing_features)
                .add_required_extension_features(dynamic_rendering_features)
                .select();

        MAG_ASSERT(phys_device_ret, "{}", phys_device_ret.error().message());

        const vkb::PhysicalDevice& physical_device = phys_device_ret.value();
        const vkb::DeviceBuilder device_builder{physical_device};
        const vkb::Result<vkb::Device> device_ret = device_builder.build();

        MAG_ASSERT(device_ret, "{}", device_ret.error().message());

        state->device = device_ret.value();

        state->disp = state->device.make_table();

        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.physicalDevice = physical_device.physical_device;
        allocator_create_info.device = state->device.device;
        allocator_create_info.instance = state->instance.instance;
        allocator_create_info.vulkanApiVersion = state->instance.api_version;
        // allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        vk_check(vmaCreateAllocator(&allocator_create_info, &state->allocator), "Failed to create memory allocator");

        // Immediate submission resources

        ICommandPoolDesc command_pool_desc = {};
        command_pool_desc.queue_type = QueueType::Graphics;
        state->immediate_command_pool_handle = create_command_pool(command_pool_desc);

        ICommandBufferDesc command_buffer_desc = {};
        command_buffer_desc.command_buffer_level = CommandBufferLevel::Primary;
        command_buffer_desc.command_pool = state->immediate_command_pool_handle;
        state->immediate_command_buffer_handle = create_command_buffer(command_buffer_desc);

        IQueueDesc queue_desc = {};
        queue_desc.queue_type = QueueType::Graphics;
        state->immediate_queue_handle = create_queue(queue_desc);

        const IFenceDesc fence_desc = {};
        state->immediate_fence_handle = create_fence(fence_desc);
    }

    void destroy_device()
    {
        wait_idle();

        vmaDestroyAllocator(state->allocator);

        vkb::destroy_device(state->device);
        vkb::destroy_surface(state->instance, state->surface);
        vkb::destroy_instance(state->instance);

        delete state;
    }

    void wait_idle() { state->disp.deviceWaitIdle(); }

    void submit_commands_immediate(const std::function<void(const CommandBufferHandle)>& function)
    {
        begin_recording_command_buffer(state->immediate_command_buffer_handle);
        function(state->immediate_command_buffer_handle);  // execute the function
        end_recording_command_buffer(state->immediate_command_buffer_handle);

        submit_queue(state->immediate_queue_handle, Invalid_ID, Invalid_ID, state->immediate_fence_handle,
                     state->immediate_command_buffer_handle);

        wait_fence(state->immediate_fence_handle, Timeout);

        reset_fence(state->immediate_fence_handle);
        reset_command_pool(state->immediate_command_pool_handle);
    }

    DescriptorLimits get_descriptor_limits()
    {
        VkPhysicalDeviceProperties2 properties = {};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        state->inst_disp.getPhysicalDeviceProperties2(state->device.physical_device, &properties);

        DescriptorLimits limits = {};

        limits.max_per_stage_combined_image_samplers = properties.properties.limits.maxPerStageDescriptorSamplers;

        limits.max_per_stage_storage_buffers = properties.properties.limits.maxPerStageDescriptorStorageBuffers;

        limits.max_per_stage_uniform_buffers = properties.properties.limits.maxPerStageDescriptorUniformBuffers;

        return limits;
    }
};  // namespace mag::gfx
