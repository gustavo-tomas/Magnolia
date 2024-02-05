#include "editor/editor_pass.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void EditorRenderPass::initialize(const uvec2& size)
    {
        auto& context = get_context();

        // The frame is rendered into this image and then copied to the swapchain
        vk::ImageUsageFlags draw_image_usage = vk::ImageUsageFlagBits::eTransferSrc |
                                               vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage |
                                               vk::ImageUsageFlagBits::eColorAttachment;

        draw_image.initialize({size.x, size.y, 1}, vk::Format::eR16G16B16A16Sfloat, draw_image_usage,
                              vk::ImageAspectFlagBits::eColor, 1, vk::SampleCountFlagBits::e1);

        this->draw_size = {size, 1};

        // Create attachments
        // -------------------------------------------------------------------------------------------------------------
        std::vector<vk::AttachmentDescription> attachments = {};
        std::vector<vk::SubpassDependency> dependencies = {};

        // Color
        vk::AttachmentDescription color_description(
            {}, draw_image.get_format(), context.get_msaa_samples(), vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDependency color_subpass_dependency(
            vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, vk::AccessFlagBits::eColorAttachmentWrite);

        vk::AttachmentReference color_reference(0, vk::ImageLayout::eColorAttachmentOptimal);

        attachments.push_back(color_description);
        dependencies.push_back(color_subpass_dependency);

        // Render pass
        vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, color_reference);

        vk::RenderPassCreateInfo render_pass_info({}, attachments, subpass, dependencies);
        vk::RenderPass render_pass = context.get_device().createRenderPass(render_pass_info);

        // Finish pass setup
        // -------------------------------------------------------------------------------------------------------------
        pass.render_pass = render_pass;

        vk::ClearValue color_clear_value({0.8f, 0.4f, 0.6f, 1.0f});
        pass.clear_values.push_back(color_clear_value);

        vk::ClearValue depth_clear_value(1.0f);
        pass.clear_values.push_back(depth_clear_value);

        // Create framebuffers and set render area
        this->on_resize(size);
    }

    void EditorRenderPass::shutdown()
    {
        auto& context = get_context();

        context.get_device().waitIdle();

        draw_image.shutdown();

        context.get_device().destroyFramebuffer(frame_buffer);
        context.get_device().destroyRenderPass(pass.render_pass);
    }

    void EditorRenderPass::before_pass(CommandBuffer& command_buffer)
    {
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    // Rendering is done during the update

    void EditorRenderPass::after_pass(CommandBuffer& command_buffer)
    {
        // Transition the draw image and the swapchain image into their correct transfer layouts
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eTransferSrcOptimal);
    }

    void EditorRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_size.x = min(size.x, draw_image.get_extent().width);
        draw_size.y = min(size.y, draw_image.get_extent().height);
        draw_size.z = 1;

        // Destroy old framebuffer
        context.get_device().destroyFramebuffer(frame_buffer);

        // Create new framebuffer
        std::vector<vk::ImageView> attachments = {};
        attachments.push_back(draw_image.get_image_view());

        vk::FramebufferCreateInfo fb_info({}, pass.render_pass, attachments, draw_size.x, draw_size.y, 1);
        frame_buffer = context.get_device().createFramebuffer(fb_info);

        // Set new render area
        pass.render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});
        pass.frame_buffer = frame_buffer;
    }
};  // namespace mag
