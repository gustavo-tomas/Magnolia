#include "renderer/render_pass.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void StandardRenderPass::initialize(const uvec2& size)
    {
        auto& context = get_context();

        // The frame is rendered into this image and then copied to the swapchain
        vk::ImageUsageFlags draw_image_usage = {};
        draw_image_usage |= vk::ImageUsageFlagBits::eTransferSrc;
        draw_image_usage |= vk::ImageUsageFlagBits::eTransferDst;
        draw_image_usage |= vk::ImageUsageFlagBits::eStorage;
        draw_image_usage |= vk::ImageUsageFlagBits::eColorAttachment;
        draw_image.initialize(vk::Extent3D(size.x, size.y, 1), vk::Format::eR16G16B16A16Sfloat, draw_image_usage,
                              vk::ImageAspectFlagBits::eColor, 1, vk::SampleCountFlagBits::e1);

        this->draw_size = {size, 1};
        this->render_scale = 0.15;

        // Create attachments
        // -------------------------------------------------------------------------------------------------------------
        std::vector<vk::AttachmentDescription> attachments = {};
        std::vector<vk::SubpassDependency> dependencies = {};

        // Color
        vk::AttachmentDescription description({}, draw_image.get_format(), context.get_msaa_samples(),
                                              vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                              vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDependency subpass_dependency(
            vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, vk::AccessFlagBits::eColorAttachmentWrite);

        vk::AttachmentReference reference(0, vk::ImageLayout::eColorAttachmentOptimal);

        attachments.push_back(description);
        dependencies.push_back(subpass_dependency);

        // Render pass
        vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, reference);

        vk::RenderPassCreateInfo render_pass_info({}, attachments, subpass, dependencies);
        vk::RenderPass render_pass = context.get_device().createRenderPass(render_pass_info);

        // Shaders
        triangle_vs.initialize("build/shaders/triangle.vert.spv", vk::ShaderStageFlagBits::eVertex);
        triangle_fs.initialize("build/shaders/triangle.frag.spv", vk::ShaderStageFlagBits::eFragment);

        // Pipeline
        triangle_pipeline.initialize(render_pass, {}, {triangle_vs, triangle_fs}, size);

        // Finish pass setup
        // -------------------------------------------------------------------------------------------------------------
        pass.pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
        pass.render_pass = render_pass;

        vk::ClearValue color_clear_value({0.2f, 0.4f, 0.6f, 1.0f});
        pass.clear_values.push_back(color_clear_value);

        vk::ClearValue depth_clear_value(1.0f);
        pass.clear_values.push_back(depth_clear_value);

        // Create framebuffers and set render area
        this->on_resize(size);
    }

    void StandardRenderPass::shutdown()
    {
        auto& context = get_context();

        draw_image.shutdown();
        triangle_pipeline.shutdown();
        triangle_vs.shutdown();
        triangle_fs.shutdown();

        context.get_device().destroyFramebuffer(frame_buffer);
        context.get_device().destroyRenderPass(pass.render_pass);
    }

    void StandardRenderPass::before_pass(CommandBuffer& command_buffer)
    {
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    void StandardRenderPass::render(const CommandBuffer& command_buffer)
    {
        const auto offset = pass.render_area.offset;
        const auto extent = pass.render_area.extent;

        const vk::Viewport viewport(0, 0, extent.width, extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor(offset, extent);

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);
        command_buffer.get_handle().bindPipeline(pass.pipeline_bind_point, triangle_pipeline.get_handle());
        command_buffer.get_handle().draw(3, 1, 0, 0);
    }

    void StandardRenderPass::after_pass(CommandBuffer& command_buffer)
    {
        auto& context = get_context();

        // Transition the draw image and the swapchain image into their correct transfer layouts
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eTransferSrcOptimal);

        command_buffer.transfer_layout(context.get_swapchain_images()[context.get_swapchain_image_index()],
                                       vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        // Copy from the draw image to the swapchain
        command_buffer.copy_image_to_image(draw_image.get_image(), {draw_size.x, draw_size.y, draw_size.z},
                                           context.get_swapchain_images()[context.get_swapchain_image_index()],
                                           vk::Extent3D(context.get_surface_extent(), 1));

        // Set swapchain image layout to Present so we can show it on the screen
        command_buffer.transfer_layout(context.get_swapchain_images()[context.get_swapchain_image_index()],
                                       vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
    }

    void StandardRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();

        draw_size.x = min(size.x, draw_image.get_extent().width) * render_scale;
        draw_size.y = min(size.y, draw_image.get_extent().height) * render_scale;
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
