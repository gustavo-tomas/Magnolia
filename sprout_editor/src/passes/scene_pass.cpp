#include "passes/scene_pass.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "editor.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/type_conversions.hpp"

namespace sprout
{
    // @TODO: temporary
    struct alignas(16) LightData
    {
            vec3 color;     // 12 bytes (3 x 4)
            f32 intensity;  // 4 bytes  (1 x 4)
            vec3 position;  // 12 bytes (3 x 4)
    };
    // @TODO: temporary

    ScenePass::ScenePass(const uvec2& size) : RenderGraphPass("ScenePass", size)
    {
        auto& app = get_application();
        auto& shader_loader = app.get_shader_manager();

        // Shaders
        mesh_shader = shader_loader.load("sprout_editor/assets/shaders/mesh_shader.mag.json");

        add_output_attachment("OutputColor", AttachmentType::Color, size);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.1, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    void ScenePass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& editor = get_editor();
        auto& scene = editor.get_active_scene();
        auto& ecs = scene.get_ecs();
        const auto& camera = scene.get_camera();

        performance_results = {};

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();
        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();

        u32 l = 0;
        LightData point_lights[LightComponent::MAX_NUMBER_OF_LIGHTS] = {};
        for (const auto& [transform, light] : light_entities)
        {
            point_lights[l++] = {light->color, light->intensity, transform->translation};
        }

        mesh_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        mesh_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        mesh_shader->set_uniform("u_global", "near_far", value_ptr(camera.get_near_far()));
        mesh_shader->set_uniform("u_global", "point_lights", &point_lights);
        mesh_shader->set_uniform("u_shader", "texture_output", &editor.get_texture_output());
        mesh_shader->set_uniform("u_shader", "normal_output", &editor.get_normal_output());

        mesh_shader->bind();

        for (u32 i = 0; i < model_entities.size(); i++)
        {
            const auto& transform = std::get<0>(model_entities[i]);
            const auto& model = std::get<1>(model_entities[i])->model;

            // @TODO: hardcoded data offset (should the shader deal with this automagically?)
            auto model_matrix = transform->get_transformation_matrix();
            mesh_shader->set_uniform("u_instance", "models", value_ptr(model_matrix), sizeof(mat4) * i);

            // Bind buffers
            command_buffer.bind_vertex_buffer(model->vbo.get_buffer());
            command_buffer.bind_index_buffer(model->ibo.get_buffer());

            for (auto& mesh : model->meshes)
            {
                // Set the material
                const auto albedo_descriptor = model->materials[mesh.material_index]->descriptor_sets[Material::Albedo];
                const auto normal_descriptor = model->materials[mesh.material_index]->descriptor_sets[Material::Normal];

                mesh_shader->bind_texture("u_albedo_texture", albedo_descriptor);
                mesh_shader->bind_texture("u_normal_texture", normal_descriptor);

                // Draw the mesh
                command_buffer.draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, i);

                performance_results.draw_calls++;
            }

            // @NOTE: not accurate but gives a good estimate
            performance_results.rendered_triangles += model->vertices.size() / 3;
        }
    }

    // PhysicsPass -----------------------------------------------------------------------------------------------------

    PhysicsPass::PhysicsPass(const uvec2& size) : RenderGraphPass("PhysicsPass", size)
    {
        auto& app = get_application();
        auto& shader_loader = app.get_shader_manager();

        // Shaders
        physics_line_shader = shader_loader.load("sprout_editor/assets/shaders/physics_line_shader.mag.json");

        add_output_attachment("OutputColor", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size, AttachmentState::Load);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.3, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    void PhysicsPass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& app = get_application();
        auto& editor = get_editor();
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& physics_engine = app.get_physics_engine();
        auto& scene = editor.get_active_scene();
        const auto& camera = scene.get_camera();

        performance_results = {};

        // Draw debug lines for physics

        physics_debug_lines.reset();
        physics_debug_lines = nullptr;
        auto& debug_lines = physics_engine.get_line_list();
        if (debug_lines.starts.empty())
        {
            return;
        }

        physics_debug_lines = std::make_unique<Line>(debug_lines.starts, debug_lines.ends, debug_lines.colors);

        physics_line_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        physics_line_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));

        physics_line_shader->bind();

        command_buffer.bind_vertex_buffer(physics_debug_lines->get_vbo().get_buffer());
        command_buffer.draw(physics_debug_lines->get_vertices().size());

        // @NOTE: not accurate but gives a good estimate
        performance_results.rendered_triangles += physics_debug_lines->get_vertices().size() / 3;
        performance_results.draw_calls++;
    }

    // GridPass --------------------------------------------------------------------------------------------------------

    GridPass::GridPass(const uvec2& size) : RenderGraphPass("GridPass", size)
    {
        auto& app = get_application();
        auto& shader_loader = app.get_shader_manager();

        // Shaders
        grid_shader = shader_loader.load("sprout_editor/assets/shaders/grid_shader.mag.json");

        add_output_attachment("OutputColor", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputDepth", AttachmentType::Depth, size, AttachmentState::Load);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.3, 0.3, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    void GridPass::on_render(RenderGraph& render_graph)
    {
        (void)render_graph;

        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& editor = get_editor();
        auto& scene = editor.get_active_scene();
        const auto& camera = scene.get_camera();

        performance_results = {};

        // Draw the grid
        grid_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        grid_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        grid_shader->set_uniform("u_global", "near_far", value_ptr(camera.get_near_far()));

        grid_shader->bind();

        command_buffer.draw(6);

        // Very accurate :)
        performance_results.rendered_triangles += 2;
        performance_results.draw_calls++;
    }
};  // namespace sprout
