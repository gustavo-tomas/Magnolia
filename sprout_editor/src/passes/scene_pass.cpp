#include "passes/scene_pass.hpp"

#include "core/application.hpp"
#include "editor.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/type_conversions.hpp"
#include "resources/material.hpp"

namespace sprout
{
    // @TODO: temporary
    struct alignas(16) LightData
    {
            vec3 color;     // 12 bytes (3 x 4)
            f32 intensity;  // 4 bytes  (1 x 4)
            vec3 position;  // 12 bytes (3 x 4)
    };

    struct alignas(16) SpriteData
    {
            mat4 model;
            vec4 size_const_face;  // Size + Constant Size + Always Face Camera
    };
    // @TODO: temporary

    ScenePass::ScenePass(const uvec2& size) : RenderGraphPass("ScenePass", size)
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        mesh_shader = shader_manager.get("sprout_editor/assets/shaders/mesh_shader.mag.json");
        sprite_shader = shader_manager.get("sprout_editor/assets/shaders/sprite_shader.mag.json");

        add_output_attachment("OutputColor", AttachmentType::Color, size);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.1, 1.0));
        pass.depth_clear_value = {1.0f};
    }

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
        mesh_shader->set_uniform("u_shader", "texture_output", &editor.get_texture_output());
        mesh_shader->set_uniform("u_shader", "normal_output", &editor.get_normal_output());

        u32 l = 0;
        const u32 number_of_lights = light_entities.size();
        mesh_shader->set_uniform("u_lights", "number_of_lights", &number_of_lights);

        for (const auto& [transform, light] : light_entities)
        {
            LightData point_light = {light->color, light->intensity, transform->translation};

            mesh_shader->set_uniform("u_lights", "lights", &point_light, sizeof(point_light) * l++);
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

    // LinePass -----------------------------------------------------------------------------------------------------

    LinePass::LinePass(const uvec2& size) : RenderGraphPass("LinePass", size)
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        line_shader = shader_manager.get("sprout_editor/assets/shaders/line_shader.mag.json");

        add_output_attachment("OutputColor", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size, AttachmentState::Load);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.3, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    LineList get_camera_gizmo(const Camera& camera);

    void LinePass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& editor = get_editor();
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& physics_engine = app.get_physics_engine();
        auto& scene = editor.get_active_scene();
        const auto& camera = scene.get_camera();

        performance_results = {};

        lines.reset(nullptr);

        LineList total_lines;

        // Get lines from AABBs

        if (editor.is_bounding_box_enabled())
        {
            const auto& model_entities =
                scene.get_ecs().get_all_components_of_types<TransformComponent, ModelComponent>();

            for (const auto& [transform, model_c] : model_entities)
            {
                for (const auto& mesh : model_c->model->meshes)
                {
                    BoundingBox mesh_aabb;
                    mesh_aabb.min = mesh.aabb_min;
                    mesh_aabb.max = mesh.aabb_max;
                    mesh_aabb = mesh_aabb.get_transformed_bounding_box(transform->get_transformation_matrix());

                    // Skip rendering if not visible
                    if (!camera.is_aabb_visible(mesh_aabb))
                    {
                        continue;
                    }

                    const auto& line_list = mesh_aabb.get_line_list(mat4(1.0f));
                    total_lines.append(line_list);
                }
            }
        }

        // Get lines from physics

        if (editor.is_physics_colliders_enabled())
        {
            const auto& physics_lines = physics_engine.get_line_list();

            if (!physics_lines.starts.empty())
            {
                total_lines.append(physics_lines);
            }
        }

        // Get lines from camera gizmos (show in editor only)

        if (scene.get_scene_state() == SceneState::Editor)
        {
            const auto& camera_entities =
                scene.get_ecs().get_all_components_of_types<TransformComponent, CameraComponent>();

            for (const auto& [transform, camera_c] : camera_entities)
            {
                total_lines.append(get_camera_gizmo(camera_c->camera));
            }
        }

        if (total_lines.starts.empty())
        {
            return;
        }

        lines = create_unique<Line>(total_lines.starts, total_lines.ends, total_lines.colors);

        line_shader->bind();

        line_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        line_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));

        // @TODO: command buffers shouldnt be accessible. They should be handled by the renderer.
        command_buffer.bind_vertex_buffer(lines->get_vbo().get_buffer());
        renderer.draw(lines->get_vertices().size());

        // @NOTE: not accurate but gives a good estimate
        performance_results.rendered_triangles += lines->get_vertices().size() / 3;
        performance_results.draw_calls++;
    }

    // Create a smaller model with fixed size to represent the camera
    LineList get_camera_gizmo(const Camera& camera)
    {
        Camera dummy_camera = camera;
        dummy_camera.set_near_far({0.1, 10.0f});

        // Get frustum corners
        std::vector<vec3> corners = dummy_camera.get_frustum().get_points();

        LineList camera_lines;

        const vec3 color = vec3(0.65, 0, 0.65);

        // L/R = Left/Right, T/B = Top/Bottom, N/F = Near/Far

        // Near quad (middle point of the diagonal of the near quad)
        const vec3 near_quad_position = (corners[3] + corners[0]) / vec3(2.0);

        // Far quad
        {
            // LBF - LTF
            camera_lines.starts.push_back(corners[4]);
            camera_lines.ends.push_back(corners[5]);
            camera_lines.colors.push_back(color);

            // LTF - RTF
            camera_lines.starts.push_back(corners[5]);
            camera_lines.ends.push_back(corners[7]);
            camera_lines.colors.push_back(color);

            // RTF - RBF
            camera_lines.starts.push_back(corners[7]);
            camera_lines.ends.push_back(corners[6]);
            camera_lines.colors.push_back(color);

            // RBF - LBF
            camera_lines.starts.push_back(corners[6]);
            camera_lines.ends.push_back(corners[4]);
            camera_lines.colors.push_back(color);
        }

        // Conections
        {
            // LBN - LBF
            camera_lines.starts.push_back(near_quad_position);
            camera_lines.ends.push_back(corners[4]);
            camera_lines.colors.push_back(color);

            // LTN - LTF
            camera_lines.starts.push_back(near_quad_position);
            camera_lines.ends.push_back(corners[5]);
            camera_lines.colors.push_back(color);

            // RTN - RTF
            camera_lines.starts.push_back(near_quad_position);
            camera_lines.ends.push_back(corners[7]);
            camera_lines.colors.push_back(color);

            // RBN - RBF
            camera_lines.starts.push_back(near_quad_position);
            camera_lines.ends.push_back(corners[6]);
            camera_lines.colors.push_back(color);
        }

        // Top triangle
        {
            const f32 top_displacement = 4.0f;
            const vec3 middle = (corners[5] + corners[7]) / vec3(2.0);
            const vec3 triangle_top_point = middle + dummy_camera.get_up() * top_displacement;
            const vec3 triangle_left_point = (corners[5] + middle) / vec3(2.0);
            const vec3 triangle_right_point = (corners[7] + middle) / vec3(2.0);

            camera_lines.starts.push_back(triangle_left_point);
            camera_lines.ends.push_back(triangle_top_point);
            camera_lines.colors.push_back(color);

            camera_lines.starts.push_back(triangle_right_point);
            camera_lines.ends.push_back(triangle_top_point);
            camera_lines.colors.push_back(color);
        }

        return camera_lines;
    }

    // GridPass --------------------------------------------------------------------------------------------------------

    GridPass::GridPass(const uvec2& size) : RenderGraphPass("GridPass", size)
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        grid_shader = shader_manager.get("sprout_editor/assets/shaders/grid_shader.mag.json");

        add_output_attachment("OutputColor", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size, AttachmentState::Load);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.3, 0.3, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    void GridPass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& editor = get_editor();
        auto& scene = editor.get_active_scene();
        const auto& camera = scene.get_camera();

        performance_results = {};

        grid_shader->bind();

        grid_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        grid_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        grid_shader->set_uniform("u_global", "near_far", value_ptr(camera.get_near_far()));

        // Draw the grid
        renderer.draw(6);

        // Very accurate :)
        performance_results.rendered_triangles += 2;
        performance_results.draw_calls++;
    }
};  // namespace sprout
