#include "passes/editor_pass.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor.hpp"
#include "editor_scene.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "physics/physics.hpp"
#include "private/renderer_type_conversions.hpp"
#include "renderer/frame.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/test_model.hpp"
#include "resources/image.hpp"

namespace sprout
{
    EditorPass::EditorPass(const uvec2& size) : RenderGraphPass("EditorPass")
    {
        add_input_attachment("OutputColor", AttachmentType::Color, size);
        add_output_attachment("EditorOutputColor", AttachmentType::Color, size);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.1, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    EditorPass::~EditorPass() = default;

    void EditorPass::on_render(RenderGraph& render_graph)
    {
        auto& context = get_context();
        auto& cmd = context.get_curr_frame().command_buffer;
        auto& scene = get_editor().get_active_scene();
        auto& ecs = scene.get_ecs();
        auto& camera = scene.get_camera();
        auto& viewport_image = render_graph.get_attachment("OutputColor");
        auto& editor = get_editor();

        // Begin
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(static_cast<SDL_Window*>(get_application().get_window().get_handle()));
        ImGui::NewFrame();

        // @NOTE: not very accurate
        performance_results = {};
        performance_results.draw_calls++;
        performance_results.rendered_triangles += 2;

        editor.render(ecs, camera, viewport_image);

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());
    }

    // GizmoPass -------------------------------------------------------------------------------------------------------

    // @TODO: temporary
    struct alignas(16) SpriteData
    {
            mat4 model;
            vec4 size_const_face;  // Size + Constant Size + Always Face Camera
    };
    // @TODO: temporary

    LineList get_camera_gizmo(const Camera& camera);

    GizmoPass::GizmoPass(const uvec2& size) : RenderGraphPass("GizmoPass")
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        line_shader = shader_manager.get("sprout_editor/assets/shaders/line_shader.mag.json");
        grid_shader = shader_manager.get("sprout_editor/assets/shaders/grid_shader.mag.json");
        sprite_shader = shader_manager.get("sprout_editor/assets/shaders/sprite_shader.mag.json");

        // Sprites
        camera_sprite = app.get_texture_manager().get("sprout_editor/assets/images/video-solid.png");
        light_sprite = app.get_texture_manager().get("sprout_editor/assets/images/lightbulb-regular.png");

        add_output_attachment("OutputColor", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size, AttachmentState::Load);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.3, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    GizmoPass::~GizmoPass() = default;

    void GizmoPass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        performance_results = {};

        render_lines();
        render_sprites();
        render_grid();
    }

    void GizmoPass::render_lines()
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& editor = get_editor();
        auto& physics_engine = app.get_physics_engine();
        auto& scene = editor.get_active_scene();
        const auto& camera = scene.get_camera();

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

        if (!scene.is_running())
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

        renderer.bind_buffers(lines.get());
        renderer.draw(lines->get_vertices().size());

        performance_results.rendered_triangles += lines->get_vertices().size() / 3;
        performance_results.draw_calls++;
    }

    void GizmoPass::render_grid()
    {
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

    void GizmoPass::render_sprites()
    {
        auto& editor = get_editor();
        auto& scene = editor.get_active_scene();
        const auto& camera = scene.get_camera();

        // Render gizmos

        sprite_shader->bind();

        sprite_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        sprite_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        sprite_shader->set_uniform("u_global", "screen_size", value_ptr(pass.size));

        u32 offset = scene.get_ecs().get_all_components_of_types<TransformComponent, SpriteComponent>().size();

        const auto& camera_entities =
            scene.get_ecs().get_all_components_of_types<TransformComponent, CameraComponent>();

        for (u32 i = 0; i < camera_entities.size(); i++)
        {
            const auto& transform = std::get<0>(camera_entities[i]);
            render_sprite(transform, camera_sprite, offset + i);
        }

        offset += camera_entities.size();

        const auto& light_entities = scene.get_ecs().get_all_components_of_types<TransformComponent, LightComponent>();

        for (u32 i = 0; i < light_entities.size(); i++)
        {
            const auto& transform = std::get<0>(light_entities[i]);
            render_sprite(transform, light_sprite, offset + i);
        }
    }

    void GizmoPass::render_sprite(TransformComponent* transform, const ref<Image>& sprite, const u32 instance)
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();

        // Remove rotation if sprite is aligned to the camera
        const vec3 model_rotation = transform->rotation;

        transform->rotation = vec3(0);

        const auto model_matrix = transform->get_transformation_matrix();
        const SpriteData sprite_data = {.model = model_matrix,
                                        .size_const_face = {sprite->width, sprite->height, true, true}};

        transform->rotation = model_rotation;

        sprite_shader->set_uniform("u_instance", "sprites", &sprite_data, sizeof(SpriteData) * instance);
        sprite_shader->set_texture("u_sprite_texture", sprite.get());

        renderer.draw(6, 1, 0, instance);

        performance_results.rendered_triangles += 2;
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
};  // namespace sprout
