#include "renderer/frame.hpp"

#include <vulkan/vulkan.hpp>

#include "core/assert.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/renderer_image.hpp"

namespace mag
{
    void FrameProvider::initialize(const u32 frame_count)
    {
        auto& context = get_context();

        frames.resize(frame_count);
        for (u64 i = 0; i < frames.size(); i++)
        {
            frames[i].render_fence = new vk::Fence();
            frames[i].render_semaphore = new vk::Semaphore();
            frames[i].present_semaphore = new vk::Semaphore();
            frames[i].command_pool = new vk::CommandPool();

            const vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlagBits::eSignaled);
            *frames[i].render_fence = context.get_device().createFence(fence_create_info);
            *frames[i].render_semaphore = context.get_device().createSemaphore({});
            *frames[i].present_semaphore = context.get_device().createSemaphore({});

            const vk::CommandPoolCreateInfo command_pool_info({}, context.get_queue_family_index());
            *frames[i].command_pool = context.get_device().createCommandPool(command_pool_info);
            frames[i].command_buffer.initialize(*frames[i].command_pool, vk::CommandBufferLevel::ePrimary);
        }
    }

    void FrameProvider::shutdown()
    {
        auto& context = get_context();
        for (auto& frame : frames)
        {
            context.get_device().destroyFence(*frame.render_fence);
            context.get_device().destroySemaphore(*frame.render_semaphore);
            context.get_device().destroySemaphore(*frame.present_semaphore);
            context.get_device().destroyCommandPool(*frame.command_pool);

            delete frame.render_fence;
            delete frame.render_semaphore;
            delete frame.present_semaphore;
            delete frame.command_pool;

            frame.render_fence = nullptr;
            frame.render_semaphore = nullptr;
            frame.present_semaphore = nullptr;
            frame.command_pool = nullptr;
        }
    }

    // Start recording commands
    b8 FrameProvider::begin_frame()
    {
        auto& context = get_context();

        Frame& curr_frame = this->get_current_frame();
        const vk::Device& device = context.get_device();

        // Acquire
        try
        {
            auto result = device.acquireNextImageKHR(context.get_swapchain(), MAG_TIMEOUT,
                                                     *curr_frame.present_semaphore, nullptr, &swapchain_image_index);

            if (result != vk::Result::eSuccess)
            {
                LOG_WARNING("Acquire next image result: '{0}'", vk::to_string(result));

                if (result == vk::Result::eErrorOutOfDateKHR)
                {
                    LOG_WARNING("Swapchain is out of date");
                    return false;
                }

                throw result;
            }
        }

        catch (const vk::OutOfDateKHRError& e)
        {
            LOG_WARNING("Swapchain is out of date");
            return false;
        }

        catch (...)
        {
            ASSERT(false, "Failed to acquire swapchain image");
        }

        VK_CHECK(device.waitForFences(*curr_frame.render_fence, true, MAG_TIMEOUT));
        device.resetFences(*curr_frame.render_fence);

        context.get_device().resetCommandPool(*curr_frame.command_pool);
        curr_frame.command_buffer.begin();

        return true;
    }

    // End command recording, submit info and acquire/present the image
    b8 FrameProvider::end_frame(const RendererImage& draw_image, const vk::Extent3D& extent)
    {
        auto& context = get_context();

        Frame& curr_frame = this->get_current_frame();
        // const vk::Device& device = context.get_device();

        // Set swapchain image layout to transfer
        curr_frame.command_buffer.transfer_layout(context.get_swapchain_images()[swapchain_image_index],
                                                  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined,
                                                  vk::ImageLayout::eTransferDstOptimal);

        // Copy from the draw image to the swapchain
        curr_frame.command_buffer.copy_image_to_image(draw_image.get_image(), extent,
                                                      context.get_swapchain_images()[swapchain_image_index],
                                                      vk::Extent3D(context.get_surface_extent(), 1));

        // Set swapchain image layout to present
        curr_frame.command_buffer.transfer_layout(context.get_swapchain_images()[swapchain_image_index],
                                                  vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal,
                                                  vk::ImageLayout::ePresentSrcKHR);

        // End command recording
        curr_frame.command_buffer.end();

        std::vector<vk::PipelineStageFlags> wait_stage = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submit;
        submit.setWaitDstStageMask(wait_stage)
            .setWaitSemaphores(*curr_frame.present_semaphore)
            .setSignalSemaphores(*curr_frame.render_semaphore)
            .setCommandBuffers(curr_frame.command_buffer.get_handle());

        context.get_graphics_queue().submit(submit, *curr_frame.render_fence);

        vk::PresentInfoKHR present_info;
        present_info.setSwapchains(context.get_swapchain())
            .setWaitSemaphores(*curr_frame.render_semaphore)
            .setImageIndices(this->swapchain_image_index);

        // Present
        try
        {
            auto result = context.get_graphics_queue().presentKHR(present_info);

            if (result != vk::Result::eSuccess) throw result;
        }

        catch (const vk::OutOfDateKHRError& e)
        {
            LOG_WARNING("Swapchain is out of date");
            return false;
        }

        catch (...)
        {
            ASSERT(false, "Failed to present swapchain image");
        }

        frame_number = (frame_number + 1) % frames.size();

        return true;
    }

    Frame& FrameProvider::get_current_frame() { return frames[frame_number]; }
    const u32& FrameProvider::get_swapchain_image_index() const { return this->swapchain_image_index; }
    const u32& FrameProvider::get_current_frame_number() const { return frame_number; }
};  // namespace mag
