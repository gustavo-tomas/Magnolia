#include "renderer/frame.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"

namespace mag
{
    void FrameProvider::initialize(const u32 frame_count)
    {
        auto& context = get_context();

        frames.resize(frame_count);
        for (u64 i = 0; i < frames.size(); i++)
        {
            vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlagBits::eSignaled);
            frames[i].render_fence = context.get_device().createFence(fence_create_info);
            frames[i].render_semaphore = context.get_device().createSemaphore({});
            frames[i].present_semaphore = context.get_device().createSemaphore({});
            frames[i].command_buffer.initialize(context.get_command_pool(), vk::CommandBufferLevel::ePrimary);
        }
    }

    void FrameProvider::shutdown()
    {
        auto& context = get_context();
        for (const auto& frame : frames)
        {
            context.get_device().destroyFence(frame.render_fence);
            context.get_device().destroySemaphore(frame.render_semaphore);
            context.get_device().destroySemaphore(frame.present_semaphore);
        }
    }

    // Start recording commands
    void FrameProvider::begin_frame()
    {
        auto& context = get_context();

        Frame& curr_frame = this->get_current_frame();
        const vk::Device& device = context.get_device();

        VK_CHECK(device.waitForFences(curr_frame.render_fence, true, MAG_TIMEOUT));
        device.resetFences(curr_frame.render_fence);

        curr_frame.command_buffer.get_handle().reset();
        curr_frame.command_buffer.begin();
    }

    // End command recording, submit info and acquire/present the image
    b8 FrameProvider::end_frame(const Image& draw_image, const vk::Extent3D& extent)
    {
        auto& context = get_context();

        Frame& curr_frame = this->get_current_frame();
        const vk::Device& device = context.get_device();

        // Acquire
        try
        {
            auto result = device.acquireNextImageKHR(context.get_swapchain(), MAG_TIMEOUT, curr_frame.present_semaphore,
                                                     nullptr, &swapchain_image_index);

            if (result != vk::Result::eSuccess) throw result;
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

        // Set swapchain image layout to transfer
        curr_frame.command_buffer.transfer_layout(context.get_swapchain_images()[swapchain_image_index],
                                                  vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        // Copy from the draw image to the swapchain
        curr_frame.command_buffer.copy_image_to_image(draw_image.get_image(), extent,
                                                      context.get_swapchain_images()[swapchain_image_index],
                                                      vk::Extent3D(context.get_surface_extent(), 1));

        // Set swapchain image layout to present
        curr_frame.command_buffer.transfer_layout(context.get_swapchain_images()[swapchain_image_index],
                                                  vk::ImageLayout::eTransferDstOptimal,
                                                  vk::ImageLayout::ePresentSrcKHR);

        // End command recording
        curr_frame.command_buffer.end();

        std::vector<vk::PipelineStageFlags> wait_stage = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submit;
        submit.setWaitDstStageMask(wait_stage)
            .setWaitSemaphores(curr_frame.present_semaphore)
            .setSignalSemaphores(curr_frame.render_semaphore)
            .setCommandBuffers(curr_frame.command_buffer.get_handle());

        context.get_graphics_queue().submit(submit, curr_frame.render_fence);

        vk::PresentInfoKHR present_info;
        present_info.setSwapchains(context.get_swapchain())
            .setWaitSemaphores(curr_frame.render_semaphore)
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
};  // namespace mag
