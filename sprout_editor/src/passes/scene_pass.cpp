#include "passes/scene_pass.hpp"

#include "core/application.hpp"
#include "editor.hpp"
#include "renderer/buffers.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/type_conversions.hpp"
#include "resources/font.hpp"
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
    // @TODO: temporary

    ScenePass::ScenePass(const uvec2& size) : RenderGraphPass("ScenePass", size)
    {
        auto& app = get_application();
        auto& shader_manager = app.get_shader_manager();

        // Shaders
        mesh_shader = shader_manager.get("sprout_editor/assets/shaders/mesh_shader.mag.json");
        sprite_shader = shader_manager.get("sprout_editor/assets/shaders/sprite_shader.mag.json");
        text_shader = shader_manager.get("sprout_editor/assets/shaders/text_shader.mag.json");

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

                // @NOTE: not accurate but gives a good estimate
                performance_results.draw_calls++;
                performance_results.rendered_triangles += mesh.index_count / 3;
            }
        }

        // Render sprites

        sprite_shader->bind();

        sprite_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        sprite_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));

        for (u32 i = 0; i < sprite_entities.size(); i++)
        {
            const auto& transform = std::get<0>(sprite_entities[i]);
            const auto& sprite = std::get<1>(sprite_entities[i]);

            const auto& sprite_tex = sprite->texture;
            const auto& sprite_quad = sprite->quad;

            const auto model_matrix = transform->get_transformation_matrix();

            // @TODO: hardcoded data offset (should the shader deal with this automagically?)
            sprite_shader->set_uniform("u_instance", "models", value_ptr(model_matrix), sizeof(mat4) * i);
            sprite_shader->set_texture("u_sprite_texture", sprite_tex.get());

            // @TODO: command buffers should be handled by the renderer
            auto& cmd = get_context().get_curr_frame().command_buffer;

            cmd.bind_vertex_buffer(sprite_quad->get_vbo().get_buffer());
            cmd.bind_index_buffer(sprite_quad->get_ibo().get_buffer());

            renderer.draw_indexed(sprite_quad->get_indices().size(), 1, 0, 0, i);

            performance_results.rendered_triangles += 2;
            performance_results.draw_calls++;
        }

        // Render text

        draw_string(0, 0);
    }

    struct TextVertex
    {
            vec3 positon;
            vec2 tex_coords;
    };

    void ScenePass::draw_string(const f64 LineSpacing, const f64 Kerning)
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& font_manager = app.get_font_manager();

        const auto& font = font_manager.get_default();

        static unique<VertexBuffer> vbo = nullptr;
        static unique<IndexBuffer> ibo = nullptr;

        std::vector<TextVertex> vertices;
        std::vector<u32> indices;

        const auto& fontGeometry = font->font_geometry;
        const auto& metrics = fontGeometry.getMetrics();
        const auto& fontAtlas = renderer.get_renderer_image(&font->atlas_image);

        f64 x = 0.0;
        f64 fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
        f64 y = 0.0;

        const f64 spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();

        f32 scale = 1;
        vec3 translation = vec3(-10, 0, 0);
        mat4 transform = translate(mat4(1.0f), translation) * math::scale(mat4(1.0f), vec3(scale));

        const f64 pixel_range = PIXEL_RANGE;

        const str text = "The Quick Brown Fox Jumps Over The Lazy Dog.\nThe Quick Brown Fox Jumps Over The Lazy Dog.";

        for (u32 i = 0; i < text.size(); i++)
        {
            char character = text[i];
            if (character == '\r') continue;

            if (character == '\n')
            {
                x = 0;
                y -= fsScale * metrics.lineHeight + LineSpacing;
                continue;
            }

            if (character == ' ')
            {
                f64 advance = spaceGlyphAdvance;
                if (i < text.size() - 1)
                {
                    char nextCharacter = text[i + 1];
                    f64 dAdvance;
                    fontGeometry.getAdvance(dAdvance, character, nextCharacter);
                    advance = (f64)dAdvance;
                }

                x += fsScale * advance + Kerning;
                continue;
            }

            if (character == '\t')
            {
                // @TODO: is this right?
                x += 4.0f * (fsScale * spaceGlyphAdvance + Kerning);
                continue;
            }

            auto glyph = fontGeometry.getGlyph(character);
            if (!glyph) glyph = fontGeometry.getGlyph('?');
            if (!glyph) return;

            f64 al, ab, ar, at;
            glyph->getQuadAtlasBounds(al, ab, ar, at);
            vec2 texCoordMin((f64)al, (f64)ab);
            vec2 texCoordMax((f64)ar, (f64)at);

            f64 pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            vec2 quadMin((f64)pl, (f64)pb);
            vec2 quadMax((f64)pr, (f64)pt);

            quadMin *= fsScale, quadMax *= fsScale;
            quadMin += vec2(x, y);
            quadMax += vec2(x, y);

            f64 texelWidth = 1.0f / fontAtlas->get_extent().width;
            f64 texelHeight = 1.0f / fontAtlas->get_extent().height;

            texCoordMin *= vec2(texelWidth, texelHeight);
            texCoordMax *= vec2(texelWidth, texelHeight);

            TextVertex v0, v1, v2, v3;

            // Bottom left
            v0.positon = transform * vec4(quadMin, 0.0f, 1.0f);
            v0.tex_coords = texCoordMin;

            // Top left
            v1.positon = transform * vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
            v1.tex_coords = {texCoordMin.x, texCoordMax.y};

            // Top right
            v2.positon = transform * vec4(quadMax, 0.0f, 1.0f);
            v2.tex_coords = texCoordMax;

            // Bottom right
            v3.positon = transform * vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
            v3.tex_coords = {texCoordMax.x, texCoordMin.y};

            u32 offset = vertices.size();

            vertices.push_back(v3);
            vertices.push_back(v2);
            vertices.push_back(v1);
            vertices.push_back(v0);

            indices.push_back(0 + offset);
            indices.push_back(1 + offset);
            indices.push_back(2 + offset);
            indices.push_back(2 + offset);
            indices.push_back(3 + offset);
            indices.push_back(0 + offset);

            if (i < text.size() - 1)
            {
                f64 advance = glyph->getAdvance();
                char nextCharacter = text[i + 1];
                fontGeometry.getAdvance(advance, character, nextCharacter);

                x += fsScale * advance + Kerning;
            }
        }

        if (!vbo)
        {
            vbo = create_unique<VertexBuffer>(vertices.data(), VEC_SIZE_BYTES(vertices));
            ibo = create_unique<IndexBuffer>(indices.data(), VEC_SIZE_BYTES(indices));
        }

        // render
        {
            auto& app = get_application();
            auto& renderer = app.get_renderer();
            auto& editor = get_editor();
            auto& scene = editor.get_active_scene();
            const auto& camera = scene.get_camera();

            text_shader->bind();

            text_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
            text_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));

            // const auto model_matrix = mat4(1.0f);

            // @TODO: hardcoded data offset (should the shader deal with this automagically?)
            // text_shader->set_uniform("u_instance", "models", value_ptr(model_matrix));

            text_shader->set_uniform("u_text_info", "color", value_ptr(vec4(1, 0, 1, 1)));
            text_shader->set_uniform("u_text_info", "pixel_range", &pixel_range);

            text_shader->set_texture("u_atlas_texture", &font->atlas_image);

            // @TODO: command buffers should be handled by the renderer
            auto& cmd = get_context().get_curr_frame().command_buffer;

            cmd.bind_vertex_buffer(vbo->get_buffer());
            cmd.bind_index_buffer(ibo->get_buffer());

            renderer.draw_indexed(indices.size());
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
