#include "renderer/render_pass.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"
#include "renderer/type_conversions.hpp"

namespace mag
{
    StandardRenderPass::StandardRenderPass(const uvec2& size)
    {
        auto& app = get_application();
        auto& shader_loader = app.get_shader_manager();
        auto& descriptors = get_context().get_descriptor_cache();

        // Set draw size before initializing images
        this->draw_size = {size, 1};

        const u32 frame_count = get_context().get_frame_count();
        draw_images.resize(frame_count);
        depth_images.resize(frame_count);
        resolve_images.resize(frame_count);

        this->initialize_images();

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

        triangle_shader =
            shader_loader.load("triangle", shader_folder + "triangle.vert.spv", shader_folder + "triangle.frag.spv");

        // Dont forget to add vertex attributes
        triangle_shader->add_attribute(vk::Format::eR32G32B32Sfloat, sizeof(Vertex::position),
                                       offsetof(Vertex, position));
        triangle_shader->add_attribute(vk::Format::eR32G32B32Sfloat, sizeof(Vertex::normal), offsetof(Vertex, normal));
        triangle_shader->add_attribute(vk::Format::eR32G32Sfloat, sizeof(Vertex::tex_coords),
                                       offsetof(Vertex, tex_coords));
        triangle_shader->add_attribute(vk::Format::eR32G32Sfloat, sizeof(Vertex::tangent), offsetof(Vertex, tangent));
        triangle_shader->add_attribute(vk::Format::eR32G32Sfloat, sizeof(Vertex::bitangent),
                                       offsetof(Vertex, bitangent));

        grid_shader = shader_loader.load("grid", shader_folder + "grid.vert.spv", shader_folder + "grid.frag.spv");

        // Descriptors
        descriptors.build_descriptors_from_shader(*triangle_shader);

        // Pipelines
        const vk::PipelineRenderingCreateInfo pipeline_create_info({}, draw_images[0].get_format(),
                                                                   depth_images[0].get_format());

        triangle_pipeline = std::make_unique<Pipeline>(pipeline_create_info, descriptors.get_descriptor_set_layouts(),
                                                       *triangle_shader);

        vk::PipelineColorBlendAttachmentState color_blend_attachment = Pipeline::default_color_blend_attachment();
        color_blend_attachment.setBlendEnable(true)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setAlphaBlendOp(vk::BlendOp::eAdd);

        grid_pipeline = std::make_unique<Pipeline>(pipeline_create_info, descriptors.get_descriptor_set_layouts(),
                                                   *grid_shader, color_blend_attachment);
    }

    StandardRenderPass::~StandardRenderPass()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

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

    void StandardRenderPass::before_render()
    {
        // Create attachments
        const vk::Rect2D render_area = vk::Rect2D({}, {draw_size.x, draw_size.y});
        const vk::ClearValue color_clear_value(vec_to_vk_clear_value(clear_color));
        const vk::ClearValue depth_clear_value(1.0f);

        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        const u32 curr_frame_number = context.get_curr_frame_number();

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

        pass.statistics = {};
    }

    void StandardRenderPass::render(const Camera& camera, ECS& ecs)
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& descriptors = context.get_descriptor_cache();
        auto& editor = get_application().get_editor();

        auto model_entities = ecs.get_components_of_entities<TransformComponent, ModelComponent>();
        auto light_entities = ecs.get_components_of_entities<TransformComponent, ModelComponent, LightComponent>();

        u32 l = 0;
        LightData point_lights[LightComponent::MAX_NUMBER_OF_LIGHTS] = {};
        for (const auto& [transform, model, light] : light_entities)
        {
            point_lights[l++] = {light->color, light->intensity, transform->translation};
        }

        triangle_shader->set_uniform_global("view", value_ptr(camera.get_view()));
        triangle_shader->set_uniform_global("projection", value_ptr(camera.get_projection()));
        triangle_shader->set_uniform_global("near_far", value_ptr(camera.get_near_far()));
        triangle_shader->set_uniform_global("point_lights", &point_lights);
        triangle_shader->set_uniform_global("texture_output", &editor.get_texture_output());
        triangle_shader->set_uniform_global("normal_output", &editor.get_normal_output());

        for (u64 b = 0; b < model_entities.size(); b++)
        {
            // @TODO: this is very wrong. b - 1 assumes that every transform has a corresponding model.
            // Because of that we have to create a model for the light, even if we intend to use a different
            // shader to render it.
            auto [transform, model] = model_entities[b];

            auto model_matrix = TransformComponent::get_transformation_matrix(*transform);
            triangle_shader->set_uniform_instance("model", value_ptr(model_matrix), b);
        }

        descriptors.bind();
        descriptors.set_offset_global(triangle_pipeline->get_layout());

        triangle_pipeline->bind();

        for (u64 m = 0; m < model_entities.size(); m++)
        {
            const auto& model = std::get<1>(model_entities[m])->model;

            descriptors.set_offset_instance(triangle_pipeline->get_layout(), m);

            // Bind buffers
            command_buffer.bind_vertex_buffer(model.vbo.get_buffer());
            command_buffer.bind_index_buffer(model.ibo.get_buffer());

            for (u64 i = 0; i < model.meshes.size(); i++)
            {
                const auto& mesh = model.meshes[i];

                // Set the material
                descriptors.set_offset_material(triangle_pipeline->get_layout(),
                                                mesh.material_index + model.albedo_descriptor_offset,
                                                TextureType::Albedo);

                descriptors.set_offset_material(triangle_pipeline->get_layout(),
                                                mesh.material_index + model.normal_descriptor_offset,
                                                TextureType::Normal);

                // Draw the mesh
                command_buffer.draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex);

                // @NOTE: not accurate but gives a good estimate
                pass.statistics.rendered_triangles += mesh.index_count;
                pass.statistics.draw_calls++;
            }
        }

        // Draw the grid
        descriptors.bind();
        descriptors.set_offset_global(grid_pipeline->get_layout());

        grid_pipeline->bind();
        command_buffer.draw(6);
    }

    void StandardRenderPass::after_render()
    {
        // Transition the draw image and the swapchain image into their correct transfer layouts
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        const u32 curr_frame_number = context.get_curr_frame_number();

        command_buffer.transfer_layout(resolve_images[curr_frame_number].get_image(),
                                       vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
    }

    void StandardRenderPass::add_model(Model& model)
    {
        auto& descriptors = get_context().get_descriptor_cache();

        model.albedo_descriptor_offset = descriptors.get_albedo_textures().size();
        model.normal_descriptor_offset = descriptors.get_normal_textures().size();
        descriptors.add_image_descriptors_for_model(model);
    }

    void StandardRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_size = {size.x * render_scale, size.y * render_scale, 1};

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
        this->on_resize(vk_extent_to_vec(context.get_surface_extent()));
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
