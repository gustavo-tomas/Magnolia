#include "passes/scene_pass.hpp"

#include "core/application.hpp"
#include "editor.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/test_model.hpp"
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
        auto text_entities = ecs.get_all_components_of_types<TransformComponent, TextComponent>();

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

        // Render text

        for (const auto& [transform, text_c] : text_entities)
        {
            draw_text(text_c, transform->get_transformation_matrix());
        }
    }

    void ScenePass::draw_text(TextComponent* text_c, const mat4& transform)
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();

        std::vector<QuadVertex> vertices;
        std::vector<u32> indices;

        const auto& font_geometry = text_c->font->font_geometry;
        const auto& metrics = font_geometry.getMetrics();
        const auto& font_atlas = renderer.get_renderer_image(&text_c->font->atlas_image);

        f64 x = 0.0;
        f64 fs_scale = 1.0 / (metrics.ascenderY - metrics.descenderY);
        f64 y = 0.0;

        const f64 space_glyph_advance = font_geometry.getGlyph(' ')->getAdvance();

        const f64 pixel_range = PIXEL_RANGE;

        for (u32 i = 0; i < text_c->text.size(); i++)
        {
            char character = text_c->text[i];
            if (character == '\r') continue;

            if (character == '\n')
            {
                x = 0;
                y -= fs_scale * metrics.lineHeight + text_c->line_spacing;
                continue;
            }

            if (character == ' ')
            {
                f64 advance = space_glyph_advance;
                if (i < text_c->text.size() - 1)
                {
                    char next_character = text_c->text[i + 1];
                    f64 d_advance;
                    font_geometry.getAdvance(d_advance, character, next_character);
                    advance = static_cast<f64>(d_advance);
                }

                x += fs_scale * advance + text_c->kerning;
                continue;
            }

            if (character == '\t')
            {
                // @TODO: is this right?
                x += 4.0f * (fs_scale * space_glyph_advance + text_c->kerning);
                continue;
            }

            auto glyph = font_geometry.getGlyph(character);
            if (!glyph) glyph = font_geometry.getGlyph('?');
            if (!glyph) return;

            f64 al, ab, ar, at;
            glyph->getQuadAtlasBounds(al, ab, ar, at);
            vec2 tex_coord_min((f64)al, (f64)ab);
            vec2 tex_coord_max((f64)ar, (f64)at);

            f64 pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            vec2 quad_min((f64)pl, (f64)pb);
            vec2 quad_max((f64)pr, (f64)pt);

            quad_min *= fs_scale, quad_max *= fs_scale;
            quad_min += vec2(x, y);
            quad_max += vec2(x, y);

            const f64 texel_width = 1.0f / font_atlas->get_extent().width;
            const f64 texel_height = 1.0f / font_atlas->get_extent().height;

            tex_coord_min *= vec2(texel_width, texel_height);
            tex_coord_max *= vec2(texel_width, texel_height);

            QuadVertex v0, v1, v2, v3;

            // Bottom left
            v0.position = transform * vec4(quad_min, 0.0f, 1.0f);
            v0.tex_coords = tex_coord_min;

            // Top left
            v1.position = transform * vec4(quad_min.x, quad_max.y, 0.0f, 1.0f);
            v1.tex_coords = {tex_coord_min.x, tex_coord_max.y};

            // Top right
            v2.position = transform * vec4(quad_max, 0.0f, 1.0f);
            v2.tex_coords = tex_coord_max;

            // Bottom right
            v3.position = transform * vec4(quad_max.x, quad_min.y, 0.0f, 1.0f);
            v3.tex_coords = {tex_coord_max.x, tex_coord_min.y};

            const u32 offset = vertices.size();

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

            if (i < text_c->text.size() - 1)
            {
                f64 advance = glyph->getAdvance();
                char next_character = text_c->text[i + 1];
                font_geometry.getAdvance(advance, character, next_character);

                x += fs_scale * advance + text_c->kerning;
            }
        }

        // @TODO: one of the ugliest hacks known to man

        renderer.upload_text(text_c, vertices, indices);

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

            text_shader->set_uniform("u_text_info", "color", value_ptr(text_c->color));
            text_shader->set_uniform("u_text_info", "pixel_range", &pixel_range);

            text_shader->set_texture("u_atlas_texture", &text_c->font->atlas_image);

            renderer.bind_buffers(text_c);

            renderer.draw_indexed(indices.size());
        }
    }
};  // namespace sprout
