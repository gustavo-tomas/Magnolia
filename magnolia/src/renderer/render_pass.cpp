#include "renderer/render_pass.hpp"

#include <filesystem>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void StandardRenderPass::initialize(const uvec2& size)
    {
        auto& context = get_context();

        // The frame is rendered into this image and then copied to the swapchain
        vk::ImageUsageFlags draw_image_usage = vk::ImageUsageFlagBits::eTransferSrc |
                                               vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage |
                                               vk::ImageUsageFlagBits::eColorAttachment;

        draw_image.initialize({size.x, size.y, 1}, vk::Format::eR16G16B16A16Sfloat, draw_image_usage,
                              vk::ImageAspectFlagBits::eColor, 1, vk::SampleCountFlagBits::e1);

        depth_image.initialize({size.x, size.y, 1}, vk::Format::eD32Sfloat, /* !TODO: hardcoded depth format */
                               vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth, 1,
                               context.get_msaa_samples());

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

        // Depth
        vk::AttachmentDescription depth_description = vk::AttachmentDescription(
            {}, depth_image.get_format(), context.get_msaa_samples(), vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDependency depth_subpass_dependency = vk::SubpassDependency(
            vk::SubpassExternal, 0,
            vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
            vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, {},
            vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        vk::AttachmentReference depth_reference =
            vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        attachments.push_back(depth_description);
        dependencies.push_back(depth_subpass_dependency);

        // Render pass
        vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, color_reference, {}, &depth_reference);

        vk::RenderPassCreateInfo render_pass_info({}, attachments, subpass, dependencies);
        vk::RenderPass render_pass = context.get_device().createRenderPass(render_pass_info);

        // Shaders
        std::filesystem::path cwd = std::filesystem::current_path();
        str last_folder;
        for (const auto& component : cwd) last_folder = component.string();

        str shader_folder = "shaders/";
        str system = "linux";

// @TODO: clean this up (maybe use a filesystem class)
#if defined(_WIN32)
        system = "windows";
#endif
        if (last_folder == "Magnolia") shader_folder = "build/" + system + "/" + shader_folder;

        triangle_vs.initialize(shader_folder + "triangle.vert.spv", vk::ShaderStageFlagBits::eVertex);
        triangle_fs.initialize(shader_folder + "triangle.frag.spv", vk::ShaderStageFlagBits::eFragment);

        // Pipeline
        triangle_pipeline.initialize(render_pass, {}, {triangle_vs, triangle_fs}, size);

        // Create a triangle mesh
        triangle.vertices.resize(3);
        triangle.vertices[0].position = {0.5f, 0.5f, 0.0f};
        triangle.vertices[1].position = {-0.5f, 0.5f, 0.0f};
        triangle.vertices[2].position = {0.0f, -0.5f, 0.0f};

        triangle.vertices[0].normal = {1.0f, 0.0f, 0.0f};
        triangle.vertices[1].normal = {0.0f, 1.0f, 0.0f};
        triangle.vertices[2].normal = {0.0f, 0.0f, 1.0f};

        triangle.vbo.initialize(triangle.vertices.data(), VECSIZE(triangle.vertices) * sizeof(Vertex),
                                context.get_allocator());

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
        depth_image.shutdown();
        triangle.vbo.shutdown();
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

    void StandardRenderPass::render(CommandBuffer& command_buffer)
    {
        const auto offset = pass.render_area.offset;
        const auto extent = pass.render_area.extent;

        const vk::Viewport viewport(0, 0, extent.width, extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor(offset, extent);

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);
        command_buffer.get_handle().bindPipeline(pass.pipeline_bind_point, triangle_pipeline.get_handle());

        command_buffer.bind_vertex_buffer(triangle.vbo.get_buffer(), 0);
        command_buffer.draw(VECSIZE(triangle.vertices), 1, 0, 0);
    }

    void StandardRenderPass::after_pass(CommandBuffer& command_buffer)
    {
        // Transition the draw image and the swapchain image into their correct transfer layouts
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eTransferSrcOptimal);
    }

    void StandardRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_size.x = min(size.x, draw_image.get_extent().width) * render_scale;
        draw_size.y = min(size.y, draw_image.get_extent().height) * render_scale;
        draw_size.z = 1;

        // Destroy old framebuffer
        context.get_device().destroyFramebuffer(frame_buffer);

        // Create new framebuffer
        std::vector<vk::ImageView> attachments = {};
        attachments.push_back(draw_image.get_image_view());
        attachments.push_back(depth_image.get_image_view());

        vk::FramebufferCreateInfo fb_info({}, pass.render_pass, attachments, draw_size.x, draw_size.y, 1);
        frame_buffer = context.get_device().createFramebuffer(fb_info);

        // Set new render area
        pass.render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});
        pass.frame_buffer = frame_buffer;
    }

    void StandardRenderPass::set_render_scale(const f32 scale)
    {
        this->render_scale = clamp(scale, 0.01f, 1.0f);
        this->on_resize({draw_image.get_extent().width, draw_image.get_extent().height});
        LOG_INFO("Render scale: {0:.2f}", render_scale);
    }
};  // namespace mag
