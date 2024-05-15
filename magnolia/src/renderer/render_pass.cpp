#include "renderer/render_pass.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    // Conversion helper
    vk::ClearValue const vec_to_vk_clear_value(const vec4& v)
    {
        const vk::ClearValue vk_clear_value({v.r, v.g, v.b, v.a});

        return vk_clear_value;
    }

    StandardRenderPass::StandardRenderPass(const uvec2& size)
    {
        auto& app = get_application();
        auto& shader_loader = app.get_shader_loader();

        // Set draw size before initializing images
        this->draw_size = {size, 1};

        const u32 frame_count = get_context().get_frame_count();
        draw_images.resize(frame_count);
        depth_images.resize(frame_count);
        resolve_images.resize(frame_count);

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

        triangle =
            shader_loader.load("triangle", shader_folder + "triangle.vert.spv", shader_folder + "triangle.frag.spv");

        // Dont forget to add vertex attributes
        triangle->add_attribute(vk::Format::eR32G32B32Sfloat, sizeof(Vertex::position), offsetof(Vertex, position));
        triangle->add_attribute(vk::Format::eR32G32B32Sfloat, sizeof(Vertex::normal), offsetof(Vertex, normal));
        triangle->add_attribute(vk::Format::eR32G32Sfloat, sizeof(Vertex::tex_coords), offsetof(Vertex, tex_coords));

        grid = shader_loader.load("grid", shader_folder + "grid.vert.spv", shader_folder + "grid.frag.spv");

        // Create descriptors layouts
        uniform_descriptors.resize(frame_count);
        image_descriptors.resize(frame_count);
        data_buffers.resize(frame_count);

        auto triangle_vs = triangle->get_modules()[0];
        auto triangle_fs = triangle->get_modules()[1];

        // @TODO: convert this to add uniforms in the shader?
        // for example: add_uniform(name);
        // the name of the uniform in the shader can be retrieved with spv reflect and then checked against the provided
        // name in the add uniform method
        for (u64 i = 0; i < frame_count; i++)
        {
            uniform_descriptors[i] = DescriptorBuilder::build_layout(triangle_vs->get_reflection(), 0);
            image_descriptors[i] = DescriptorBuilder::build_layout(triangle_fs->get_reflection(), 2);
        }

        // Descriptor layouts
        const std::vector<vk::DescriptorSetLayout> descriptor_set_layouts = {
            uniform_descriptors[0].layout, uniform_descriptors[0].layout, image_descriptors[0].layout};

        // Pipelines
        const vk::PipelineRenderingCreateInfo pipeline_create_info({}, draw_images[0].get_format(),
                                                                   depth_images[0].get_format());

        triangle_pipeline =
            std::make_unique<Pipeline>(pipeline_create_info, descriptor_set_layouts, *triangle, draw_size);

        vk::PipelineColorBlendAttachmentState color_blend_attachment = Pipeline::default_color_blend_attachment();
        color_blend_attachment.setBlendEnable(true)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setAlphaBlendOp(vk::BlendOp::eAdd);

        grid_pipeline = std::make_unique<Pipeline>(pipeline_create_info, descriptor_set_layouts, *grid, draw_size,
                                                   color_blend_attachment);

        // Initialize global buffer
        add_uniform_data(sizeof(GlobalData));
    }

    StandardRenderPass::~StandardRenderPass()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        if (!data_buffers.empty())
        {
            for (auto& descriptor : uniform_descriptors)
            {
                descriptor.buffer.unmap_memory();
                descriptor.buffer.shutdown();
            }
        }

        if (!textures.empty())
        {
            for (auto& descriptor : image_descriptors)
            {
                descriptor.buffer.unmap_memory();
                descriptor.buffer.shutdown();
            }
        }

        for (auto& buffers : data_buffers)
        {
            for (auto& buffer : buffers)
            {
                buffer.shutdown();
            }
        }

        for (auto& image : draw_images)
        {
            image.shutdown();
        }

        for (auto& image : depth_images)
        {
            image.shutdown();
        }

        for (auto& image : resolve_images)
        {
            image.shutdown();
        }
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

        const u32 frame_count = context.get_frame_count();

        for (u32 i = 0; i < frame_count; i++)
        {
            draw_images[i].initialize({draw_size.x, draw_size.y, 1}, vk::Format::eR16G16B16A16Sfloat, draw_image_usage,
                                      vk::ImageAspectFlagBits::eColor, 1, context.get_msaa_samples());

            depth_images[i].initialize({draw_size.x, draw_size.y, 1}, context.get_supported_depth_format(),
                                       vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth,
                                       1, context.get_msaa_samples());

            resolve_images[i].initialize({draw_size.x, draw_size.y, 1}, vk::Format::eR16G16B16A16Sfloat,
                                         resolve_image_usage, vk::ImageAspectFlagBits::eColor, 1,
                                         vk::SampleCountFlagBits::e1);
        }
    }

    void StandardRenderPass::before_render(CommandBuffer& command_buffer)
    {
        // Create attachments
        const vk::Rect2D render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});
        const vk::ClearValue color_clear_value(vec_to_vk_clear_value(clear_color));
        const vk::ClearValue depth_clear_value(1.0f);

        const u32 curr_frame_number = get_context().get_curr_frame_number();

        // @TODO: check attachments load/store ops
        pass.color_attachment = vk::RenderingAttachmentInfo(
            draw_images[curr_frame_number].get_image_view(), vk::ImageLayout::eColorAttachmentOptimal,
            vk::ResolveModeFlagBits::eAverage, resolve_images[curr_frame_number].get_image_view(),
            vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
            color_clear_value);

        pass.depth_attachment = vk::RenderingAttachmentInfo(
            depth_images[curr_frame_number].get_image_view(), vk::ImageLayout::eDepthStencilAttachmentOptimal,
            vk::ResolveModeFlagBits::eNone, {}, {}, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
            depth_clear_value);

        pass.rendering_info =
            vk::RenderingInfo({}, render_area, 1, {}, pass.color_attachment, &pass.depth_attachment, {});

        const vk::Viewport viewport(0, 0, render_area.extent.width, render_area.extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor = render_area;

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);

        command_buffer.transfer_layout(resolve_images[curr_frame_number].get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    void StandardRenderPass::render(CommandBuffer& command_buffer, const Camera& camera, ECS& ecs)
    {
        auto& context = get_context();
        const u32 curr_frame_number = context.get_curr_frame_number();

        auto model_entities = ecs.get_components_of_entities<TransformComponent, ModelComponent>();
        auto light_entities = ecs.get_components_of_entities<TransformComponent, ModelComponent, LightComponent>();

        LightData point_lights[LightComponent::MAX_NUMBER_OF_LIGHTS] = {};

        u32 l = 0;
        for (const auto& [transform, model, light] : light_entities)
        {
            point_lights[l++] = {light->color, light->intensity, transform->translation, 0};
        }

        for (u64 b = 0; b < data_buffers[curr_frame_number].size(); b++)
        {
            // Global Data
            if (b == 0)
            {
                GlobalData global_data = {.view = camera.get_view(),
                                          .projection = camera.get_projection(),
                                          .near_far = camera.get_near_far(),

                                          .gamer_padding_dont_use_this_is_just_for_padding_gamer_gaming_game = {},

                                          .point_lights = {}};

                memcpy(global_data.point_lights, point_lights, sizeof(global_data.point_lights));

                data_buffers[curr_frame_number][b].copy(&global_data, data_buffers[curr_frame_number][b].get_size());

                continue;
            }

            // @TODO: this is very wrong. b - 1 assumes that every transform has a corresponding model.
            // Because of that we have to create a model for the light, even if we intend to use a different
            // shader to render it.
            // Models
            auto [transform, model] = model_entities[b - 1];
            const mat4 model_matrix = TransformComponent::get_transformation_matrix(*transform);

            data_buffers[curr_frame_number][b].copy(value_ptr(model_matrix),
                                                    data_buffers[curr_frame_number][b].get_size());
        }

        // The pipeline layout should be the same for both pipelines
        std::vector<vk::DescriptorBufferBindingInfoEXT> descriptor_buffer_binding_infos;

        if (!data_buffers.empty())
        {
            descriptor_buffer_binding_infos.push_back(
                {uniform_descriptors[curr_frame_number].buffer.get_device_address(),
                 vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT});
        }

        if (!textures.empty())
        {
            descriptor_buffer_binding_infos.push_back({image_descriptors[curr_frame_number].buffer.get_device_address(),
                                                       vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                                                           vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT});
        }

        // Bind descriptor buffers and set offsets
        command_buffer.get_handle().bindDescriptorBuffersEXT(descriptor_buffer_binding_infos);

        const auto pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
        const u32 buffer_indices = 0;
        const u32 image_indices = 1;
        u64 buffer_offsets = 0;

        // Global matrices (set 0)
        command_buffer.get_handle().setDescriptorBufferOffsetsEXT(pipeline_bind_point, triangle_pipeline->get_layout(),
                                                                  0, buffer_indices, buffer_offsets);

        triangle_pipeline->bind(command_buffer);

        u64 tex_idx = 0;
        auto models = ecs.get_components<ModelComponent>();
        for (u64 m = 0; m < models.size(); m++)
        {
            const auto& model = models[m]->model;

            // Model matrices (set 1)
            buffer_offsets = (m + 1) * uniform_descriptors[curr_frame_number].size;
            command_buffer.get_handle().setDescriptorBufferOffsetsEXT(
                pipeline_bind_point, triangle_pipeline->get_layout(), 1, buffer_indices, buffer_offsets);

            for (u64 i = 0; i < model.meshes.size(); i++)
            {
                const auto& mesh = model.meshes[i];

                // @TODO: only one offset can be set
                // Images (set 2)
                if (!mesh.textures.empty())
                {
                    buffer_offsets = tex_idx * image_descriptors[curr_frame_number].size;
                    command_buffer.get_handle().setDescriptorBufferOffsetsEXT(
                        pipeline_bind_point, triangle_pipeline->get_layout(), 2, image_indices, buffer_offsets);

                    tex_idx++;
                }

                // Draw the mesh
                command_buffer.bind_vertex_buffer(mesh.vbo.get_buffer(), 0);
                command_buffer.bind_index_buffer(mesh.ibo.get_buffer(), 0);
                command_buffer.draw_indexed(VECSIZE(mesh.indices), 1, 0, 0, 0);
            }
        }

        // Draw the grid
        grid_pipeline->bind(command_buffer);
        command_buffer.draw(6, 1, 0, 0);
    }

    void StandardRenderPass::after_render(CommandBuffer& command_buffer)
    {
        // Transition the draw image and the swapchain image into their correct transfer layouts
        const u32 curr_frame_number = get_context().get_curr_frame_number();

        command_buffer.transfer_layout(resolve_images[curr_frame_number].get_image(),
                                       vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
    }

    void StandardRenderPass::add_uniform_data(const u64 buffer_size)
    {
        auto& context = get_context();

        const u32 frame_count = context.get_frame_count();

        // Delete old descriptor buffers
        if (!data_buffers.empty() && !data_buffers.begin()->empty())
        {
            for (auto& descriptor : uniform_descriptors)
            {
                descriptor.buffer.unmap_memory();
                descriptor.buffer.shutdown();
            }
        }

        // Create descriptor buffer that holds global data and model transform
        for (u32 i = 0; i < frame_count; i++)
        {
            Buffer buffer;
            buffer.initialize(buffer_size,
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                              VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

            data_buffers[i].push_back(buffer);

            uniform_descriptors[i].buffer.initialize(
                data_buffers[i].size() * uniform_descriptors[i].size,
                VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

            uniform_descriptors[i].buffer.map_memory();

            DescriptorBuilder::build(uniform_descriptors[i], data_buffers[i]);
        }
    }

    void StandardRenderPass::add_uniform_texture(const Model& model)
    {
        auto& context = get_context();

        const u32 frame_count = context.get_frame_count();

        // Delete old descriptor buffers
        if (!textures.empty())
        {
            for (auto& descriptor : image_descriptors)
            {
                descriptor.buffer.unmap_memory();
                descriptor.buffer.shutdown();
            }
        }

        // Put all models textures in a single array
        for (auto& mesh : model.meshes)
        {
            for (auto& texture : mesh.textures)
            {
                textures.push_back(*texture);
            }
        }

        // Create descriptor buffer that holds texture data
        for (u32 i = 0; i < frame_count; i++)
        {
            image_descriptors[i].buffer.initialize(
                textures.size() * image_descriptors[i].size,
                VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

            image_descriptors[i].buffer.map_memory();

            DescriptorBuilder::build(image_descriptors[i], textures);
        }
    }

    void StandardRenderPass::add_model(const Model& model)
    {
        this->add_uniform_data(sizeof(InstanceData));
        this->add_uniform_texture(model);
    }

    void StandardRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_size.x = size.x * render_scale;
        draw_size.y = size.y * render_scale;
        draw_size.z = 1;

        for (auto& image : draw_images)
        {
            image.shutdown();
        }

        for (auto& image : depth_images)
        {
            image.shutdown();
        }

        for (auto& image : resolve_images)
        {
            image.shutdown();
        }

        this->initialize_images();
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

    const Image& StandardRenderPass::get_target_image() const
    {
        const u32 curr_frame_number = get_context().get_curr_frame_number();

        return resolve_images[curr_frame_number];
    };
};  // namespace mag
