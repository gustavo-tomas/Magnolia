#include "renderer/frame.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

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
            frames[i].command_buffer.initialize(context.get_command_pool(), vk::CommandBufferLevel::ePrimary);
        }
    }

    void FrameProvider::shutdown()
    {
        for (const auto& frame : frames) get_context().get_device().destroyFence(frame.render_fence);
    }

    void FrameProvider::begin_frame()
    {
        auto& context = get_context();

        // Reset fences, acquire image and start recording commands
        Frame& curr_frame = this->get_current_frame();
        const vk::Device& device = context.get_device();

        VK_CHECK(device.waitForFences(curr_frame.render_fence, true, MAG_TIMEOUT));
        device.resetFences(curr_frame.render_fence);

        context.get_command_buffer().get_handle().reset();

        try
        {
            auto result = device.acquireNextImageKHR(context.get_swapchain(), MAG_TIMEOUT,
                                                     context.get_present_semaphore(), nullptr, &swapchain_image_index);

            if (result != vk::Result::eSuccess) throw result;
        }

        catch (const vk::OutOfDateKHRError& e)
        {
            LOG_WARNING("Swapchain is out of date");
            return;
        }

        catch (...)
        {
            ASSERT(false, "Failed to acquire swapchain image");
        }

        curr_frame.command_buffer.begin();
    }

    void FrameProvider::end_frame()
    {
        auto& context = get_context();

        // End command recording, submit info and present the image
        Frame& curr_frame = this->get_current_frame();

        // Transition the image layout to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        vk::ImageMemoryBarrier imageBarrier;
        imageBarrier.setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
            .setImage(context.get_swapchain_images()[swapchain_image_index])
            .setSrcAccessMask({})
            .setDstAccessMask({})
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        curr_frame.command_buffer.get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                               vk::PipelineStageFlagBits::eBottomOfPipe, {}, nullptr,
                                                               nullptr, imageBarrier);

        curr_frame.command_buffer.end();

        std::vector<vk::PipelineStageFlags> wait_stage = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submit;
        submit.setWaitDstStageMask(wait_stage)
            .setWaitSemaphores(context.get_present_semaphore())
            .setSignalSemaphores(context.get_render_semaphore())
            .setCommandBuffers(curr_frame.command_buffer.get_handle());

        context.get_graphics_queue().submit(submit, curr_frame.render_fence);

        vk::PresentInfoKHR present_info;
        present_info.setSwapchains(context.get_swapchain())
            .setWaitSemaphores(context.get_render_semaphore())
            .setImageIndices(this->swapchain_image_index);

        try
        {
            auto result = context.get_graphics_queue().presentKHR(present_info);

            if (result != vk::Result::eSuccess) throw result;
        }

        catch (const vk::OutOfDateKHRError& e)
        {
            LOG_WARNING("Swapchain is out of date");
            return;
        }

        catch (...)
        {
            ASSERT(false, "Failed to present swapchain image");
        }

        frame_number = (frame_number + 1) % frames.size();
    }
};  // namespace mag
