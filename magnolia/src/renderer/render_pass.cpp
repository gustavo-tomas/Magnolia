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
                                               vk::ImageUsageFlagBits::eColorAttachment |
                                               vk::ImageUsageFlagBits::eSampled;

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

        triangle_vs.initialize(shader_folder + "triangle.vert.spv");
        triangle_fs.initialize(shader_folder + "triangle.frag.spv");

        grid_vs.initialize(shader_folder + "grid.vert.spv");
        grid_fs.initialize(shader_folder + "grid.frag.spv");

        {
            // @TODO: Create one camera buffer and descriptor set per frame
            camera_buffer.initialize(sizeof(CameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, context.get_allocator());

            auto& descriptor_cache = context.get_descriptor_cache();
            auto& descriptor_allocator = context.get_descriptor_allocator();

            // Descriptor set allocate info
            const vk::DescriptorBufferInfo descriptor_buffer_info(camera_buffer.get_buffer(), 0, sizeof(CameraData));

            // Create descriptor sets
            ASSERT(DescriptorBuilder::begin(&descriptor_cache, &descriptor_allocator)
                       .bind(triangle_vs.get_reflection(), &descriptor_buffer_info)
                       .build(descriptor_set, set_layout),
                   "Failed to build descriptor set");
        }

        // Pipelines
        triangle_pipeline.initialize(render_pass, {set_layout}, {triangle_vs, triangle_fs},
                                     Vertex::get_vertex_description(), size);

        vk::PipelineColorBlendAttachmentState color_blend_attachment = Pipeline::default_color_blend_attachment();
        color_blend_attachment.setBlendEnable(true)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setAlphaBlendOp(vk::BlendOp::eAdd);

        grid_pipeline.initialize(render_pass, {set_layout}, {grid_vs, grid_fs}, {}, size, color_blend_attachment);

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

        camera_buffer.shutdown();
        draw_image.shutdown();
        depth_image.shutdown();
        triangle_pipeline.shutdown();
        grid_pipeline.shutdown();
        triangle_vs.shutdown();
        triangle_fs.shutdown();
        grid_vs.shutdown();
        grid_fs.shutdown();

        context.get_device().destroyFramebuffer(frame_buffer);
        context.get_device().destroyRenderPass(pass.render_pass);
    }

    void StandardRenderPass::before_pass(CommandBuffer& command_buffer)
    {
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    void StandardRenderPass::render(CommandBuffer& command_buffer, const Mesh& mesh)
    {
        const auto offset = pass.render_area.offset;
        const auto extent = pass.render_area.extent;

        const vk::Viewport viewport(0, 0, extent.width, extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor(offset, extent);

        const CameraData camera_data = {
            .view = camera->get_view(), .projection = camera->get_projection(), .near_far = camera->get_near_far()};
        camera_buffer.copy(&camera_data, sizeof(CameraData));

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);

        // The pipeline layout should be the same for both pipelines
        command_buffer.get_handle().bindDescriptorSets(pass.pipeline_bind_point, triangle_pipeline.get_layout(), 0,
                                                       descriptor_set, {});

        // Draw the mesh
        command_buffer.get_handle().bindPipeline(pass.pipeline_bind_point, triangle_pipeline.get_handle());
        command_buffer.bind_vertex_buffer(mesh.vbo.get_buffer(), 0);
        command_buffer.draw(VECSIZE(mesh.vertices), 1, 0, 0);

        // Draw the grid
        command_buffer.get_handle().bindPipeline(pass.pipeline_bind_point, grid_pipeline.get_handle());
        command_buffer.draw(6, 1, 0, 0);
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

    void StandardRenderPass::set_camera(Camera* camera) { this->camera = camera; }
};  // namespace mag
