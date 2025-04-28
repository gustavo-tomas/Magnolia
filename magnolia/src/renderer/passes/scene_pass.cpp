#include "scene_pass.hpp"

#include "../assets/shaders/include/common.h"
#include "ecs/components.hpp"
#include "ecs/ecs.hpp"
#include "math/types.hpp"
#include "private/renderer_type_conversions.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "resources/font.hpp"
#include "resources/material.hpp"
#include "resources/model.hpp"
#include "scene/scene.hpp"

namespace mag
{
    DepthPrePass::DepthPrePass(const uvec2& size) : RenderGraphPass("DepthPrePass")
    {
        // Shaders
        depth_prepass_shader = resource::get_shader(MAG_ASSET_DIR "shaders/depth_prepass_shader.mag.json");

        add_output_attachment("OutputDepth", AttachmentType::DepthStencil, size);

        pass.size = size;
        pass.color_clear_value = vec4(0.0, 1.0, 1.0, 1.0);
        pass.depth_stencil_clear_value = vec2(1.0f, 1.0f);
    }

    DepthPrePass::~DepthPrePass() = default;

    void DepthPrePass::on_render(RenderGraph& render_graph, Scene& scene)
    {
        (void)render_graph;

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

            gfx::bind_buffers(model.get());

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
                gfx::draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, i);

                performance_results.draw_calls++;
                performance_results.rendered_triangles += mesh.index_count / 3;
            }
        }
    }

    ScenePass::ScenePass(const uvec2& size) : RenderGraphPass("ScenePass")
    {
        // Shaders
        mesh_shader = resource::get_shader(MAG_ASSET_DIR "shaders/mesh_shader.mag.json");
        sprite_shader = resource::get_shader(MAG_ASSET_DIR "shaders/sprite_shader.mag.json");
        text_shader = resource::get_shader(MAG_ASSET_DIR "shaders/text_shader.mag.json");

        // @TODO: we are skipping the depth prepass for now, until we need to process many lights with forward+
        // add_input_attachment("OutputDepth", AttachmentType::DepthStencil, size, AttachmentState::Load);

        add_output_attachment("OutputColorScene", AttachmentType::Color, size);
        add_output_attachment("OutputDepth", AttachmentType::DepthStencil, size);

        pass.size = size;
        pass.color_clear_value = vec4(0.1, 0.1, 0.1, 1.0);
        pass.depth_stencil_clear_value = vec2(1.0f, 1.0f);
    }

    ScenePass::~ScenePass() = default;

    void ScenePass::on_render(RenderGraph& render_graph, Scene& scene)
    {
        (void)render_graph;

        auto& ecs = scene.get_ecs();
        const auto& camera = scene.get_camera();

        performance_results = {};

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();
        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();
        auto sprite_entities = ecs.get_all_components_of_types<TransformComponent, SpriteComponent>();
        auto text_entities = ecs.get_all_components_of_types<TransformComponent, TextComponent>();

        // Render models

        // @TODO: move debug outputs to some editor shader
        const u32 texture_output = 0;

        mesh_shader->bind();

        mesh_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        mesh_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        mesh_shader->set_uniform("u_global", "near_far", value_ptr(camera.get_near_far()));
        mesh_shader->set_uniform("u_push_constants", "texture_output", &texture_output);
        mesh_shader->set_uniform("u_push_constants", "normal_output", &texture_output);

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

            gfx::bind_buffers(model.get());

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
                    const auto& material = resource::get_material(model->materials[mesh.material_index]);

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
                gfx::draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, i);

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

            gfx::draw(4, 1, 0, i);

            performance_results.rendered_triangles += 2;
            performance_results.draw_calls++;
        }

        // Render text

        text_shader->bind();

        text_shader->set_uniform("u_global", "view", value_ptr(camera.get_view()));
        text_shader->set_uniform("u_global", "projection", value_ptr(camera.get_projection()));
        text_shader->set_uniform("u_global", "screen_size", value_ptr(pass.size));

        u32 data_offset = 0;
        for (u32 i = 0; i < text_entities.size(); i++)
        {
            const auto& transform = std::get<0>(text_entities[i]);
            const auto& text = std::get<1>(text_entities[i]);

            // Skip fonts that are not loaded yet
            if (text->font->loading_status != LoadingStatus::UploadedToGpu)
            {
                continue;
            }

            const f32 scale = transform->scale.x;
            f32 x = transform->translation.x;
            f32 y = transform->translation.y;
            f32 z = transform->translation.z;

            for (auto& c : text->text)
            {
                Character& ch = text->font->characters[c];

                // Skip chars with no visual representation (i.e. spaces)
                if (ch.data.empty())
                {
                    x += (ch.advance.x >> 6) * scale;
                    continue;
                }

                // Format newlines
                if (c == '\n')
                {
                    y -= ch.size.y * 1.5 * scale;  // @TODO: hardcoded line spacing
                    x = transform->translation.x;
                    continue;
                }

                // Don't offset the first letter of the text
                const f32 xpos = x + (data_offset > 0 ? ch.bearing.x * scale : 0);
                const f32 ypos = y - (data_offset > 0 ? (ch.size.y - ch.bearing.y) * scale : 0);
                const f32 zpos = z;

                TransformComponent char_transform;
                char_transform.translation = vec3(xpos, ypos, zpos);
                char_transform.scale = vec3(ch.size.x * scale, ch.size.y * scale, 1.0f);
                char_transform.rotation = transform->rotation;

                // @TODO: rotation is a bit iffy but for now its ok
                const mat4 model_matrix = char_transform.get_transformation_matrix();

                TextData text_data;
                text_data.color = text->color;
                text_data.model = model_matrix;

                text_shader->set_uniform("u_instance", "texts", &text_data, sizeof(TextData) * data_offset);
                text_shader->set_texture("u_char_texture", &ch.texture);

                gfx::draw(4, 1, 0, data_offset);
                data_offset++;

                performance_results.rendered_triangles += 2;
                performance_results.draw_calls++;

                // Advance cursors for next glyph (note that advance is number of 1/64 pixels) bitshift by 6 to get
                // value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
                x += (ch.advance.x >> 6) * scale;
            }
        }
    }

    PostProcessingPass::PostProcessingPass(const uvec2& size) : RenderGraphPass("PostPass")
    {
        // Shaders
        post_shader = resource::get_shader(MAG_ASSET_DIR "shaders/post_shader.mag.json");

        add_input_attachment("OutputColorScene", AttachmentType::Color, size, AttachmentState::Load);
        add_output_attachment("OutputColor", AttachmentType::Color, size);

        pass.size = size;
        pass.color_clear_value = vec4(0.1, 0.1, 0.1, 1.0);
        pass.depth_stencil_clear_value = vec2(1.0f, 1.0f);
    }

    PostProcessingPass::~PostProcessingPass() = default;

    void PostProcessingPass::on_render(RenderGraph& render_graph, Scene& scene)
    {
        (void)scene;

        performance_results = {};

        // Only apply post processing to the final combined result
        const b8 apply_tonemapping = true;
        post_shader->set_uniform("u_push_constants", "apply_tonemapping", &apply_tonemapping);

        auto& screen_color = render_graph.get_attachment("OutputColorScene");

        post_shader->bind();
        post_shader->set_texture("u_screen_color_texture", &screen_color);

        gfx::draw(4);

        performance_results.rendered_triangles += 2;
        performance_results.draw_calls++;
    }
};  // namespace mag
