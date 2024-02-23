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
        this->pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
        this->render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});

        // Create attachments and rendering info
        this->on_resize(size);

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
        const vk::PipelineRenderingCreateInfo pipeline_create_info({}, draw_image.get_format(),
                                                                   depth_image.get_format());

        triangle_pipeline.initialize(pipeline_create_info, {set_layout}, {triangle_vs, triangle_fs},
                                     Vertex::get_vertex_description(), size);

        vk::PipelineColorBlendAttachmentState color_blend_attachment = Pipeline::default_color_blend_attachment();
        color_blend_attachment.setBlendEnable(true)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setAlphaBlendOp(vk::BlendOp::eAdd);

        grid_pipeline.initialize(pipeline_create_info, {set_layout}, {grid_vs, grid_fs}, {}, size,
                                 color_blend_attachment);
    }

    void StandardRenderPass::shutdown()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        camera_buffer.shutdown();
        draw_image.shutdown();
        depth_image.shutdown();
        triangle_pipeline.shutdown();
        grid_pipeline.shutdown();
        triangle_vs.shutdown();
        triangle_fs.shutdown();
        grid_vs.shutdown();
        grid_fs.shutdown();
    }

    void StandardRenderPass::before_render(CommandBuffer& command_buffer)
    {
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    void StandardRenderPass::render(CommandBuffer& command_buffer, const Mesh& mesh)
    {
        const vk::Viewport viewport(0, 0, render_area.extent.width, render_area.extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor(render_area.offset, render_area.extent);

        const CameraData camera_data = {
            .view = camera->get_view(), .projection = camera->get_projection(), .near_far = camera->get_near_far()};
        camera_buffer.copy(&camera_data, sizeof(CameraData));

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);

        // The pipeline layout should be the same for both pipelines
        command_buffer.get_handle().bindDescriptorSets(pipeline_bind_point, triangle_pipeline.get_layout(), 0,
                                                       descriptor_set, {});

        // Draw the mesh
        command_buffer.get_handle().bindPipeline(pipeline_bind_point, triangle_pipeline.get_handle());
        command_buffer.bind_vertex_buffer(mesh.vbo.get_buffer(), 0);
        command_buffer.draw(VECSIZE(mesh.vertices), 1, 0, 0);

        // Draw the grid
        command_buffer.get_handle().bindPipeline(pipeline_bind_point, grid_pipeline.get_handle());
        command_buffer.draw(6, 1, 0, 0);
    }

    void StandardRenderPass::after_render(CommandBuffer& command_buffer)
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

        render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});

        delete pass.rendering_info;
        delete pass.color_attachment;
        delete pass.depth_attachment;

        // Create attachments
        const vk::ClearValue color_clear_value({0.2f, 0.4f, 0.6f, 1.0f});
        const vk::ClearValue depth_clear_value(1.0f);

        // @TODO: check attachments load/store ops
        pass.color_attachment = new vk::RenderingAttachmentInfo(
            draw_image.get_image_view(), vk::ImageLayout::eColorAttachmentOptimal, vk::ResolveModeFlagBits::eNone, {},
            {}, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, color_clear_value);

        pass.depth_attachment = new vk::RenderingAttachmentInfo(
            depth_image.get_image_view(), vk::ImageLayout::eDepthStencilAttachmentOptimal,
            vk::ResolveModeFlagBits::eNone, {}, {}, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
            depth_clear_value);

        pass.rendering_info =
            new vk::RenderingInfo({}, render_area, 1, {}, *pass.color_attachment, pass.depth_attachment, {});
    }

    void StandardRenderPass::set_render_scale(const f32 scale)
    {
        this->render_scale = clamp(scale, 0.01f, 1.0f);
        this->on_resize({draw_image.get_extent().width, draw_image.get_extent().height});
        LOG_INFO("Render scale: {0:.2f}", render_scale);
    }

    void StandardRenderPass::set_camera(Camera* camera) { this->camera = camera; }
};  // namespace mag
