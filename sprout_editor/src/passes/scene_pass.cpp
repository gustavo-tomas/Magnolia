#include "passes/scene_pass.hpp"

#include "../assets/shaders/include/common.h"
#include "core/application.hpp"
#include "editor.hpp"
#include "editor_scene.hpp"
#include "math/generic.hpp"
#include "math/type_definitions.hpp"
#include "private/renderer_type_conversions.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "resources/image.hpp"
#include "resources/material.hpp"
#include "resources/model.hpp"

namespace sprout
{
    DepthPrePass::DepthPrePass(const uvec2& size) : RenderGraphPass("DepthPrePass")
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        depth_prepass_shader = shader_manager.get("sprout_editor/assets/shaders/depth_prepass_shader.mag.json");

        add_output_attachment("OutputDepth", AttachmentType::DepthStencil, size);

        pass.size = size;
        pass.color_clear_value = vec4(0.0, 1.0, 1.0, 1.0);
        pass.depth_stencil_clear_value = vec2(1.0f, 1.0f);
    }

    DepthPrePass::~DepthPrePass() = default;

    void DepthPrePass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& editor = get_editor();
        auto& scene = editor.get_active_scene();
        auto& ecs = scene.get_ecs();
        const auto& camera = scene.get_camera();

        performance_results = {};

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();

        // Render models

        depth_prepass_shader->bind();

        depth_prepass_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        depth_prepass_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        depth_prepass_shader->set_uniform("u_global", "near_far", value_ptr(camera.get_near_far()));

        for (u32 i = 0; i < model_entities.size(); i++)
        {
            const auto& transform = std::get<0>(model_entities[i]);
            const auto& model = std::get<1>(model_entities[i])->model;

            // @TODO: hardcoded data offset (should the shader deal with this automagically?)
            const auto& model_matrix = transform->get_transformation_matrix();
            depth_prepass_shader->set_uniform("u_instance", "models", value_ptr(model_matrix), sizeof(mat4) * i);

            renderer.bind_buffers(model.get());

            for (auto& mesh : model->meshes)
            {
                // @TODO: improve AABB calculation performance. I think its not a terrible ideia to apply the transform
                // and use a dirty flag to recalculate the bounding box.

                // Calculate transformed aabbs
                BoundingBox mesh_aabb;
                mesh_aabb.min = mesh.aabb_min;
                mesh_aabb.max = mesh.aabb_max;
                mesh_aabb = mesh_aabb.get_transformed_bounding_box(model_matrix);

                // Skip rendering if not visible
                if (!camera.is_aabb_visible(mesh_aabb))
                {
                    continue;
                }

                // Draw the mesh
                renderer.draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, i);

                performance_results.draw_calls++;
                performance_results.rendered_triangles += mesh.index_count / 3;
            }
        }
    }

    ScenePass::ScenePass(const uvec2& size) : RenderGraphPass("ScenePass")
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        mesh_shader = shader_manager.get("sprout_editor/assets/shaders/mesh_shader.mag.json");
        sprite_shader = shader_manager.get("sprout_editor/assets/shaders/sprite_shader.mag.json");

        add_input_attachment("OutputDepth", AttachmentType::DepthStencil, size, AttachmentState::Load);

        add_output_attachment("OutputColorScene", AttachmentType::Color, size);
        add_output_attachment("OutputDepth", AttachmentType::DepthStencil, size, AttachmentState::Load);

        pass.size = size;
        pass.color_clear_value = vec4(0.1, 0.1, 0.1, 1.0);
        pass.depth_stencil_clear_value = vec2(1.0f, 1.0f);
    }

    ScenePass::~ScenePass() = default;

    void ScenePass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& material_manager = app.get_material_manager();
        auto& editor = get_editor();
        auto& scene = editor.get_active_scene();
        auto& ecs = scene.get_ecs();
        const auto& camera = scene.get_camera();

        performance_results = {};

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();
        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();
        auto sprite_entities = ecs.get_all_components_of_types<TransformComponent, SpriteComponent>();

        // Render models

        mesh_shader->bind();

        mesh_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        mesh_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        mesh_shader->set_uniform("u_global", "near_far", value_ptr(camera.get_near_far()));
        mesh_shader->set_uniform("u_push_constants", "texture_output", &editor.get_texture_output());
        mesh_shader->set_uniform("u_push_constants", "normal_output", &editor.get_normal_output());

        u32 l = 0;
        const u32 number_of_lights = light_entities.size();
        mesh_shader->set_uniform("u_push_constants", "number_of_lights", &number_of_lights);

        for (const auto& [transform, light] : light_entities)
        {
            LightData point_light = {light->color, light->intensity, transform->translation};

            mesh_shader->set_uniform("u_lights", "lights", &point_light, sizeof(point_light) * l++);
        }

        // Set light uniforms so vulkan stops complaining about unbound descriptor sets
        if (number_of_lights == 0)
        {
            static const LightData dummy_light = {.color = vec3(0), .intensity = 0, .position = vec3(0)};
            static const u32 num_lights = 1;

            mesh_shader->set_uniform("u_push_constants", "number_of_lights", &num_lights);
            mesh_shader->set_uniform("u_lights", "lights", &dummy_light);
        }

        for (u32 i = 0; i < model_entities.size(); i++)
        {
            const auto& transform = std::get<0>(model_entities[i]);
            const auto& model = std::get<1>(model_entities[i])->model;

            // @TODO: hardcoded data offset (should the shader deal with this automagically?)
            const auto& model_matrix = transform->get_transformation_matrix();
            mesh_shader->set_uniform("u_instance", "models", value_ptr(model_matrix), sizeof(mat4) * i);

            renderer.bind_buffers(model.get());

            i32 last_material_idx = -1;
            for (auto& mesh : model->meshes)
            {
                // @TODO: improve AABB calculation performance. I think its not a terrible ideia to apply the transform
                // and use a dirty flag to recalculate the bounding box.

                // Calculate transformed aabbs
                BoundingBox mesh_aabb;
                mesh_aabb.min = mesh.aabb_min;
                mesh_aabb.max = mesh.aabb_max;
                mesh_aabb = mesh_aabb.get_transformed_bounding_box(model_matrix);

                // Skip rendering if not visible
                if (!camera.is_aabb_visible(mesh_aabb))
                {
                    continue;
                }

                // Set the material. The meshes are sorted by material index (see model loader), so we draw all meshes
                // with the same material before swapping to the next one.
                if (last_material_idx != static_cast<i32>(mesh.material_index))
                {
                    last_material_idx = mesh.material_index;
                    const auto& material = material_manager.get(model->materials[mesh.material_index]);

                    // @TODO: hardcoded material parameters
                    static MaterialData material_data;
                    material_data.albedo = vec4(1, 1, 1, 1);
                    material_data.roughness = 1;
                    material_data.metallic = 1;

                    mesh_shader->set_uniform("u_push_constants", "material_index", &mesh.material_index);
                    mesh_shader->set_uniform("u_material", "materials", &material_data,
                                             sizeof(MaterialData) * mesh.material_index);
                    mesh_shader->set_material("u_material_textures", material.get());
                }

                // Draw the mesh
                renderer.draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, i);

                performance_results.draw_calls++;
                performance_results.rendered_triangles += mesh.index_count / 3;
            }
        }

        // Render sprites

        sprite_shader->bind();

        sprite_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        sprite_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        sprite_shader->set_uniform("u_global", "screen_size", value_ptr(pass.size));

        for (u32 i = 0; i < sprite_entities.size(); i++)
        {
            const auto& transform = std::get<0>(sprite_entities[i]);
            const auto& sprite = std::get<1>(sprite_entities[i]);

            // Remove rotation if sprite is aligned to the camera
            const vec3 model_rotation = transform->rotation;
            if (sprite->always_face_camera)
            {
                transform->rotation = vec3(0);
            }

            const auto& sprite_tex = sprite->texture;

            const auto model_matrix = transform->get_transformation_matrix();
            const SpriteData sprite_data = {.model = model_matrix,
                                            .size_const_face = {sprite->texture->width, sprite->texture->height,
                                                                sprite->constant_size, sprite->always_face_camera}};

            transform->rotation = model_rotation;

            sprite_shader->set_uniform("u_instance", "sprites", &sprite_data, sizeof(SpriteData) * i);
            sprite_shader->set_texture("u_sprite_texture", sprite_tex.get());

            renderer.draw(6, 1, 0, i);

            performance_results.rendered_triangles += 2;
            performance_results.draw_calls++;
        }
    }

    PostProcessingPass::PostProcessingPass(const uvec2& size) : RenderGraphPass("PostPass")
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        post_shader = shader_manager.get("sprout_editor/assets/shaders/post_shader.mag.json");

        add_input_attachment("OutputColorScene", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputColor", AttachmentType::Color, size);

        pass.size = size;
        pass.color_clear_value = vec4(0.1, 0.1, 0.1, 1.0);
        pass.depth_stencil_clear_value = vec2(1.0f, 1.0f);
    }

    PostProcessingPass::~PostProcessingPass() = default;

    void PostProcessingPass::on_render(RenderGraph& render_graph)
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& editor = get_editor();

        performance_results = {};

        // Only apply post processing to the final combined result
        const b8 apply_tonemapping = editor.get_texture_output() == 0;
        post_shader->set_uniform("u_push_constants", "apply_tonemapping", &apply_tonemapping);

        auto& screen_color = render_graph.get_attachment("OutputColorScene");

        post_shader->bind();
        post_shader->set_texture("u_screen_color_texture", &screen_color);

        renderer.draw(6);

        performance_results.rendered_triangles += 2;
        performance_results.draw_calls++;
    }
};  // namespace sprout
