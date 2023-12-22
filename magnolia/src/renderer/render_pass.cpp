#include "renderer/render_pass.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void StandardRenderPass::initialize(const uvec2& size)
    {
        auto& context = get_context();

        // Create attachments
        // -------------------------------------------------------------------------------------------------------------
        std::vector<vk::AttachmentDescription> attachments = {};
        std::vector<vk::SubpassDependency> dependencies = {};

        // Color
        vk::AttachmentDescription description({}, context.get_swapchain_image_format(), context.get_msaa_samples(),
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
        triangle_vs.create("build/shaders/triangle.vert.spv", vk::ShaderStageFlagBits::eVertex);
        triangle_fs.create("build/shaders/triangle.frag.spv", vk::ShaderStageFlagBits::eFragment);

        // Pipeline
        triangle_pipeline.create(render_pass, {}, {triangle_vs, triangle_fs}, size);

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

        triangle_pipeline.destroy();
        triangle_vs.destroy();
        triangle_fs.destroy();

        for (const auto& frame_buffer : frame_buffers) context.get_device().destroyFramebuffer(frame_buffer);

        context.get_device().destroyRenderPass(pass.render_pass);
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

    void StandardRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();

        // Destroy old framebuffers
        for (const auto& frame_buffer : frame_buffers) context.get_device().destroyFramebuffer(frame_buffer);
        frame_buffers.clear();

        // Create framebuffers
        const u64 image_count = context.get_swapchain_images().size();
        this->frame_buffers.reserve(image_count);
        for (u32 i = 0; i < image_count; i++)
        {
            std::vector<vk::ImageView> attachments = {};
            attachments.push_back(context.get_swapchain_image_views()[i]);

            vk::FramebufferCreateInfo fb_info({}, pass.render_pass, attachments, size.x, size.y, 1);
            this->frame_buffers.push_back(context.get_device().createFramebuffer(fb_info));
        }

        // Set new render area
        pass.render_area = vk::Rect2D({}, {size.x, size.y});
    }

    Pass& StandardRenderPass::get_pass()
    {
        auto& context = get_context();

        // @TODO: figure a better way to resolve the framebuffer
        this->pass.frame_buffer = this->frame_buffers[context.get_swapchain_image_index()];
        return pass;
    }
};  // namespace mag
