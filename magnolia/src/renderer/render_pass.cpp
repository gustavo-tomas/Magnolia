#include "renderer/render_pass.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"

namespace mag
{
    void StandardRenderPass::initialize(const uvec2& size)
    {
        // Set draw size before initializing images
        this->draw_size = {size, 1};
        this->pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
        this->render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});

        this->initialize_images();

        // Create attachments and rendering info
        this->on_resize(size);

        // Shaders
        const std::filesystem::path cwd = std::filesystem::current_path();
        const str last_folder = cwd.filename().string();
        str system = "linux";
        str shader_folder = "shaders/";

// @TODO: clean this up (maybe use a filesystem class)
#if defined(_WIN32)
        system = "windows";
#endif
        if (last_folder == "Magnolia") shader_folder = "build/" + system + "/" + shader_folder;

        triangle_vs.initialize(shader_folder + "triangle.vert.spv");
        triangle_fs.initialize(shader_folder + "triangle.frag.spv");

        grid_vs.initialize(shader_folder + "grid.vert.spv");
        grid_fs.initialize(shader_folder + "grid.frag.spv");

        // Create descriptors layouts
        uniform_descriptor = DescriptorBuilder::build_layout(triangle_vs.get_reflection(), 0);
        image_descriptor = DescriptorBuilder::build_layout(triangle_fs.get_reflection(), 2);

        // Descriptor layouts
        const std::vector<vk::DescriptorSetLayout> descriptor_set_layouts = {
            uniform_descriptor.layout, uniform_descriptor.layout, image_descriptor.layout};

        // Pipelines
        const vk::PipelineRenderingCreateInfo pipeline_create_info({}, draw_image.get_format(),
                                                                   depth_image.get_format());

        triangle_pipeline.initialize(pipeline_create_info, descriptor_set_layouts, {triangle_vs, triangle_fs},
                                     Vertex::get_vertex_description(), draw_size);

        vk::PipelineColorBlendAttachmentState color_blend_attachment = Pipeline::default_color_blend_attachment();
        color_blend_attachment.setBlendEnable(true)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setAlphaBlendOp(vk::BlendOp::eAdd);

        grid_pipeline.initialize(pipeline_create_info, descriptor_set_layouts, {grid_vs, grid_fs}, {}, draw_size,
                                 color_blend_attachment);
    }

    void StandardRenderPass::shutdown()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        for (auto& buffer : data_buffers) buffer.shutdown();

        uniform_descriptor.buffer.unmap_memory();
        uniform_descriptor.buffer.shutdown();

        image_descriptor.buffer.unmap_memory();
        image_descriptor.buffer.shutdown();

        draw_image.shutdown();
        depth_image.shutdown();
        resolve_image.shutdown();
        triangle_pipeline.shutdown();
        grid_pipeline.shutdown();
        triangle_vs.shutdown();
        triangle_fs.shutdown();
        grid_vs.shutdown();
        grid_fs.shutdown();
    }

    void StandardRenderPass::initialize_images()
    {
        auto& context = get_context();

        // The frame is rendered into this image and then copied to the swapchain
        const vk::ImageUsageFlags draw_image_usage =
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

        const vk::ImageUsageFlags resolve_image_usage =
            vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

        draw_image.initialize({draw_size.x, draw_size.y, 1}, vk::Format::eR16G16B16A16Sfloat, draw_image_usage,
                              vk::ImageAspectFlagBits::eColor, 1, context.get_msaa_samples());

        depth_image.initialize({draw_size.x, draw_size.y, 1},
                               vk::Format::eD32Sfloat, /* !TODO: hardcoded depth format */
                               vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth, 1,
                               context.get_msaa_samples());

        resolve_image.initialize({draw_size.x, draw_size.y, 1}, vk::Format::eR16G16B16A16Sfloat, resolve_image_usage,
                                 vk::ImageAspectFlagBits::eColor, 1, vk::SampleCountFlagBits::e1);
    }

    void StandardRenderPass::before_render(CommandBuffer& command_buffer)
    {
        command_buffer.transfer_layout(resolve_image.get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    void StandardRenderPass::render(CommandBuffer& command_buffer, const Camera& camera,
                                    const std::vector<Model>& models)
    {
        const vk::Viewport viewport(0, 0, render_area.extent.width, render_area.extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor(render_area.offset, render_area.extent);

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);

        const CameraData camera_data = {
            .view = camera.get_view(), .projection = camera.get_projection(), .near_far = camera.get_near_far()};
        data_buffers[0].copy(&camera_data, data_buffers[0].get_size());

        for (u64 b = 1; b < data_buffers.size(); b++)
        {
            const auto& model = models[b - 1];

            const quat pitch = angleAxis(radians(model.rotation.x), vec3(1.0f, 0.0f, 0.0f));
            const quat yaw = angleAxis(radians(model.rotation.y), vec3(0.0f, 1.0f, 0.0f));
            const quat roll = angleAxis(radians(model.rotation.z), vec3(0.0f, 0.0f, 1.0f));

            const mat4 rotation_matrix = toMat4(roll) * toMat4(yaw) * toMat4(pitch);
            const mat4 translation_matrix = translate(mat4(1.0f), model.position);
            const mat4 scale_matrix = scale(mat4(1.0f), model.scale);

            const mat4 model_matrix = translation_matrix * rotation_matrix * scale_matrix;

            data_buffers[b].copy(value_ptr(model_matrix), data_buffers[b].get_size());
        }

        // The pipeline layout should be the same for both pipelines
        std::vector<vk::DescriptorBufferBindingInfoEXT> descriptor_buffer_binding_infos;

        descriptor_buffer_binding_infos.push_back(
            {uniform_descriptor.buffer.get_device_address(), vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT});

        descriptor_buffer_binding_infos.push_back(
            {image_descriptor.buffer.get_device_address(), vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                                                               vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT});

        // Bind descriptor buffers and set offsets
        command_buffer.get_handle().bindDescriptorBuffersEXT(descriptor_buffer_binding_infos);

        const u32 buffer_indices = 0;
        const u32 image_indices = 1;
        u64 buffer_offsets = 0;

        // Global matrices (set 0)
        command_buffer.get_handle().setDescriptorBufferOffsetsEXT(pipeline_bind_point, triangle_pipeline.get_layout(),
                                                                  0, buffer_indices, buffer_offsets);

        command_buffer.get_handle().bindPipeline(pipeline_bind_point, triangle_pipeline.get_handle());

        u64 tex_idx = 0;
        for (u64 m = 0; m < models.size(); m++)
        {
            const auto& model = models[m];

            // Model matrices (set 1)
            buffer_offsets = (m + 1) * uniform_descriptor.size;
            command_buffer.get_handle().setDescriptorBufferOffsetsEXT(
                pipeline_bind_point, triangle_pipeline.get_layout(), 1, buffer_indices, buffer_offsets);

            for (u64 i = 0; i < model.meshes.size(); i++)
            {
                const auto& mesh = model.meshes[i];

                // @TODO: only one offset can be set
                // Images (set 2)
                if (!mesh.textures.empty())
                {
                    buffer_offsets = tex_idx * image_descriptor.size;
                    command_buffer.get_handle().setDescriptorBufferOffsetsEXT(
                        pipeline_bind_point, triangle_pipeline.get_layout(), 2, image_indices, buffer_offsets);

                    tex_idx++;
                }

                // Draw the mesh
                command_buffer.bind_vertex_buffer(mesh.vbo.get_buffer(), 0);
                command_buffer.bind_index_buffer(mesh.ibo.get_buffer(), 0);
                command_buffer.draw_indexed(VECSIZE(mesh.indices), 1, 0, 0, 0);
            }
        }

        // Draw the grid
        command_buffer.get_handle().bindPipeline(pipeline_bind_point, grid_pipeline.get_handle());
        command_buffer.draw(6, 1, 0, 0);
    }

    void StandardRenderPass::after_render(CommandBuffer& command_buffer)
    {
        // Transition the draw image and the swapchain image into their correct transfer layouts
        command_buffer.transfer_layout(resolve_image.get_image(), vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eTransferSrcOptimal);
    }

    void StandardRenderPass::set_camera() { add_uniform(sizeof(CameraData)); }

    void StandardRenderPass::add_uniform(const u64 buffer_size)
    {
        // Delete old descriptor buffers
        if (data_buffers.size() > 0)
        {
            uniform_descriptor.buffer.unmap_memory();
            uniform_descriptor.buffer.shutdown();
        }

        // @TODO: Create one camera buffer and descriptor set per frame
        Buffer buffer;
        buffer.initialize(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                          VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        data_buffers.push_back(buffer);

        // Create descriptor buffer that holds global data and model transform
        uniform_descriptor.buffer.initialize(
            data_buffers.size() * uniform_descriptor.size,
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        uniform_descriptor.buffer.map_memory();

        DescriptorBuilder::build(uniform_descriptor, data_buffers);
    }

    void StandardRenderPass::add_model(const Model& model)
    {
        this->add_uniform(sizeof(ModelData));

        // Delete old descriptor buffers
        if (textures.size() > 0)
        {
            image_descriptor.buffer.unmap_memory();
            image_descriptor.buffer.shutdown();
        }

        // Put all models textures in a single array
        for (auto& mesh : model.meshes)
            for (auto& texture : mesh.textures) textures.push_back(*texture);

        // Create descriptor buffer that holds texture data
        image_descriptor.buffer.initialize(
            textures.size() * image_descriptor.size,
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        image_descriptor.buffer.map_memory();

        DescriptorBuilder::build(image_descriptor, textures);
    }

    void StandardRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_size.x = size.x * render_scale;
        draw_size.y = size.y * render_scale;
        draw_size.z = 1;

        draw_image.shutdown();
        depth_image.shutdown();
        resolve_image.shutdown();

        this->initialize_images();

        render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});

        delete pass.rendering_info;
        delete pass.color_attachment;
        delete pass.depth_attachment;

        // Create attachments
        const vk::ClearValue color_clear_value({0.2f, 0.4f, 0.6f, 1.0f});
        const vk::ClearValue depth_clear_value(1.0f);

        // @TODO: check attachments load/store ops
        pass.color_attachment = new vk::RenderingAttachmentInfo(
            draw_image.get_image_view(), vk::ImageLayout::eColorAttachmentOptimal, vk::ResolveModeFlagBits::eAverage,
            resolve_image.get_image_view(), vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore, color_clear_value);

        pass.depth_attachment = new vk::RenderingAttachmentInfo(
            depth_image.get_image_view(), vk::ImageLayout::eDepthStencilAttachmentOptimal,
            vk::ResolveModeFlagBits::eNone, {}, {}, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
            depth_clear_value);

        pass.rendering_info =
            new vk::RenderingInfo({}, render_area, 1, {}, *pass.color_attachment, pass.depth_attachment, {});
    }

    void StandardRenderPass::set_render_scale(const f32 scale)
    {
        auto& context = get_context();

        this->render_scale = clamp(scale, 0.01f, 1.0f);
        this->on_resize({context.get_surface_extent().width, context.get_surface_extent().height});
        LOG_INFO("Render scale: {0:.2f}", render_scale);

        // Dont forget to set editor viewport image
        get_application().get_editor().set_viewport_image(this->get_target_image());
    }
};  // namespace mag
