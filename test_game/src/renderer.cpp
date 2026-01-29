#include "renderer.hpp"

#include <magnolia/core/logger.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/gfx/gfx.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/file_system.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/material.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/shader.hpp>
#include <magnolia/resources/texture.hpp>

#include "components.hpp"
#include "scene.hpp"

// @TODO: temp
#include "../assets/shaders/include/common.h"

// This is the game renderer. It sits one layer above the gfx frontend and manages shaders, buffers and textures.

namespace game
{
#define MESH_SHADER "test_game/assets/shaders/mesh_shader.mag.json"
#define SPRITE_SHADER "test_game/assets/shaders/sprite_shader.mag.json"
#define TEXT_SHADER "test_game/assets/shaders/text_shader.mag.json"
#define FLOOR_SHADER "test_game/assets/shaders/floor_shader.mag.json"
#define LINE_SHADER "test_game/assets/shaders/line_shader.mag.json"
#define DEBUG_TEXT_SHADER "test_game/assets/shaders/debug_text_shader.mag.json"

    Renderer::Renderer()
    {
        // Load shaders

        build_shader(MESH_SHADER, false);
        build_shader(SPRITE_SHADER, false);
        build_shader(TEXT_SHADER, false);

        // Debug

        build_shader(FLOOR_SHADER, false);
        build_shader(LINE_SHADER, false);
        build_shader(DEBUG_TEXT_SHADER, false);
    }

    Renderer::~Renderer() = default;

    void Renderer::render_scene(Scene& scene, const f32 dt)
    {
        if (!mag::gfx::begin_frame())
        {
            return;
        }

        render_sprites(scene);
        render_models(scene);
        render_text(scene);
        render_debug(scene, dt);

        if (!mag::gfx::end_frame())
        {
            return;
        }
    }

    void Renderer::on_event(const mag::Event& e) { mag::gfx::on_event(e); }

    void Renderer::render_models(Scene& scene)
    {
        auto& ecs = scene.get_ecs();
        const auto& camera = scene.get_camera();

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();

        if (model_entities.empty())
        {
            return;
        }

        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();

        // Render models

        mag::gfx::use_shader(shaders[MESH_SHADER]);

        // Global buffer
        {
            struct GlobalData
            {
                    mat4 view;
                    mat4 projection;
                    u32 light_count;
            };

            static GlobalData global_data = {};
            global_data.view = camera.get_view();
            global_data.projection = camera.get_projection();
            global_data.light_count = light_entities.size();

            mag::gfx::set_uniform("u_global", &global_data);
        }

        // Lights buffer
        {
            u32 light_num = 0;
            for (const auto& [transform, light] : light_entities)
            {
                LightData light_data = {};
                light_data.position = transform->translation;
                light_data.color = light->color;
                light_data.intensity = light->intensity;

                mag::gfx::set_uniform("u_light", &light_data, light_num++);
            }
        }

        u32 mesh_offset = 0;
        u32 material_offset = 0;
        u32 texture_offset = 0;

        static MeshData mesh_data = {};
        mesh_data.material_idx = Max_U32;

        static std::unordered_map<str, mag::gfx::TextureHandle> texture_handles;
        static std::unordered_map<str, mag::gfx::VertexBufferHandle> vertex_buffer_handles;
        static std::unordered_map<str, mag::gfx::IndexBufferHandle> index_buffer_handles;

        for (auto& model_entity : model_entities)
        {
            const auto& transform = std::get<0>(model_entity);
            const auto& model = std::get<1>(model_entity)->model;

            if (model->loading_status != LoadingStatus::Finished)
            {
                continue;
            }

            const str& model_name = model->file_path;
            if (!vertex_buffer_handles.contains(model_name))
            {
                const mag::gfx::VertexBufferHandle vertex_buffer =
                    mag::gfx::create_vertex_buffer(VEC_SIZE_BYTES(model->vertices), model->vertices.data());

                const mag::gfx::IndexBufferHandle index_buffer =
                    mag::gfx::create_index_buffer(VEC_SIZE_BYTES(model->indices), model->indices.data());

                vertex_buffer_handles[model_name] = vertex_buffer;
                index_buffer_handles[model_name] = index_buffer;
            }

            mag::gfx::bind_vertex_buffer(vertex_buffer_handles[model_name]);
            mag::gfx::bind_index_buffer(index_buffer_handles[model_name]);

            for (auto& mesh : model->meshes)
            {
                // Instance
                mesh_data.model = transform->get_transformation_matrix();

                // Set the material. The meshes are sorted by material index (see model loader), so we draw all
                // meshes with the same material before swapping to the next one.
                if (mesh_data.material_idx != mesh.material_index)
                {
                    mesh_data.material_idx = mesh.material_index;

                    const ref<mag::MaterialResource>& material = model->materials[mesh.material_index];

                    for (const auto& [slot, texture] : material->textures)
                    {
                        const str& name = texture->file_path;

                        if (!texture_handles.contains(name))
                        {
                            texture_handles[name] = mag::gfx::create_texture(
                                texture->width, texture->height, texture->pixels.size(), texture->pixels.data());
                        }
                    }

                    // @TODO: hardcoded material parameters
                    static MaterialData material_data = {};
                    material_data.albedo = vec4(1, 1, 1, 1);
                    material_data.roughness = 1;
                    material_data.metallic = 1;
                    material_data.albedo_tex_idx = texture_offset;
                    material_data.normal_tex_idx = texture_offset + 1;
                    material_data.roughness_tex_idx = texture_offset + 2;
                    material_data.metalness_tex_idx = texture_offset + 3;

                    mag::gfx::set_uniform("u_material", &material_data, material_offset);

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material->textures.at(TextureSlot::Albedo)->file_path],
                                          texture_offset);

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material->textures.at(TextureSlot::Normal)->file_path],
                                          texture_offset + 1);

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material->textures.at(TextureSlot::Roughness)->file_path],
                                          texture_offset + 2);  // ARM texture

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material->textures.at(TextureSlot::Metalness)->file_path],
                                          texture_offset + 2);  // ARM texture

                    material_offset++;
                    texture_offset += 4;
                }

                mag::gfx::set_uniform("u_instance", &mesh_data, mesh_offset);

                // Draw the mesh
                mag::gfx::draw_indexed(mesh.index_count, 1, mesh.base_index, static_cast<i32>(mesh.base_vertex),
                                       mesh_offset);

                mesh_offset++;
            }
        }
    }

    void Renderer::render_sprites(Scene& scene)
    {
        auto sprite_entities = scene.get_ecs().get_all_components_of_types<TransformComponent, SpriteComponent>();

        if (sprite_entities.empty())
        {
            return;
        }

        mag::Camera& camera = scene.get_camera();

        mag::gfx::use_shader(shaders[SPRITE_SHADER]);

        // Global buffer
        {
            struct GlobalData
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalData global_data = {};
            global_data.view = camera.get_view();
            global_data.projection = camera.get_projection();

            mag::gfx::set_uniform("u_global", &global_data);
        }

        u32 texture_offset = 0;
        for (auto& sprite_entity : sprite_entities)
        {
            const auto& transform = std::get<0>(sprite_entity);
            const auto& sprite = std::get<1>(sprite_entity);

            // Skip sprites that are not loaded yet
            if (sprite->texture->loading_status != LoadingStatus::Finished)
            {
                continue;
            }

            // @TODO: temp
            static std::unordered_map<str, mag::gfx::TextureHandle> texture_handles;

            const str& name = sprite->texture->file_path;

            if (!texture_handles.contains(sprite->texture->file_path))
            {
                const ref<mag::TextureResource>& texture = sprite->texture;

                texture_handles[name] = mag::gfx::create_texture(texture->width, texture->height,
                                                                 texture->pixels.size(), texture->pixels.data());
            }

            // Remove rotation if sprite is aligned to the camera
            const quat model_rotation = transform->rotation;
            if (sprite->always_face_camera)
            {
                transform->rotation = quat();
            }

            const mat4 model_matrix = transform->get_transformation_matrix();
            SpriteData sprite_data = {};

            sprite_data.model = model_matrix;
            sprite_data.size_const_face = {sprite->texture->width, sprite->texture->height, sprite->constant_size,
                                           sprite->always_face_camera};
            sprite_data.texture_idx = texture_offset;

            transform->rotation = model_rotation;

            // Instance
            mag::gfx::set_uniform("u_instance", &sprite_data, texture_offset);

            // Textures
            mag::gfx::set_uniform("u_sprite_textures", texture_handles[name], texture_offset);

            texture_offset++;
        }

        mag::gfx::draw(4, texture_offset);
    }

    void Renderer::render_text(Scene& scene)
    {
        auto text_entities = scene.get_ecs().get_all_components_of_types<TransformComponent, TextComponent>();

        if (text_entities.empty())
        {
            return;
        }

        mag::gfx::use_shader(shaders[TEXT_SHADER]);

        mag::Camera& camera = scene.get_camera();

        // @TODO: temp
        struct FontData
        {
                std::unordered_map<c8, mag::gfx::TextureHandle> char_texture_handles;
                u32 idx;
        };

        static std::unordered_map<str, FontData> fonts;

        struct GlobalData
        {
                mat4 view;
                mat4 projection;
        };

        static GlobalData global_data = {};
        global_data.view = camera.get_view();
        global_data.projection = camera.get_projection();

        mag::gfx::set_uniform("u_global", &global_data);

        u32 char_offset = 0;
        for (auto& text_entity : text_entities)
        {
            const auto& transform = std::get<0>(text_entity);
            const auto& text = std::get<1>(text_entity);

            // Skip fonts that are not loaded yet
            if (text->font->loading_status != LoadingStatus::Finished)
            {
                continue;
            }

            const str& name = text->font->file_path;

            if (!fonts.contains(name))
            {
                const ref<mag::FontResource>& font = text->font;

                FontData font_data = {};
                font_data.idx = fonts.size();  // The index is used to map a letter to the correct texture (and font)

                for (const auto& [c, ch] : font->characters)
                {
                    // Skip non visual characters
                    if (ch.texture.pixels.empty())
                    {
                        continue;
                    }

                    font_data.char_texture_handles[c] =
                        mag::gfx::create_texture(ch.texture.width, ch.texture.height, ch.texture.pixels.size(),
                                                 ch.texture.pixels.data(), mag::gfx::Format::R8_UNORM);
                }

                fonts[name] = font_data;
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
                    x += static_cast<f32>(ch.advance.x >> 6) * scale;
                    continue;
                }

                // Format newlines
                if (c == '\n')
                {
                    y -= static_cast<f32>(ch.size.y) * 1.5f * scale;  // @TODO: hardcoded line spacing
                    x = transform->translation.x;
                    continue;
                }

                // Don't offset the first letter of the text
                const f32 xpos = x + (char_offset > 0 ? static_cast<f32>(ch.bearing.x) * scale : 0);
                const f32 ypos = y - (char_offset > 0 ? static_cast<f32>(ch.size.y - ch.bearing.y) * scale : 0);
                const f32 zpos = z;

                TransformComponent char_transform;
                char_transform.translation = vec3(xpos, ypos, zpos);
                char_transform.scale =
                    vec3(static_cast<f32>(ch.size.x) * scale, static_cast<f32>(ch.size.y) * scale, 1.0f);
                char_transform.rotation = transform->rotation;

                // @TODO: rotation is a bit iffy but for now its ok
                const mat4 model_matrix = char_transform.get_transformation_matrix();

                const u32 font_offset = fonts[name].idx * 128;  // skip next 128 character textures
                const u32 texture_idx = font_offset + c;

                TextData text_data = {};
                text_data.color = text->color;
                text_data.model = model_matrix;
                text_data.texture_idx = texture_idx;

                mag::gfx::set_uniform("u_instance", &text_data, char_offset);

                mag::gfx::set_uniform("u_char_textures", fonts[name].char_texture_handles[c], texture_idx);

                char_offset++;

                // Advance cursors for next glyph (note that advance is number of 1/64 pixels) bitshift by 6 to
                // get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
                x += static_cast<f32>(ch.advance.x >> 6) * scale;
            }
        }

        mag::gfx::draw(4, char_offset);
    }

    void Renderer::render_debug(Scene& scene, const f32 dt)
    {
        mag::Camera& camera = scene.get_camera();

        OrthographicCameraDesc ortho_camera_desc = {};
        ortho_camera_desc.near = -100.0f;
        ortho_camera_desc.far = 100.0f;
        ortho_camera_desc.position = vec3(0.0f);
        ortho_camera_desc.rotation = vec3(0.0f);
        ortho_camera_desc.size = 1000.0f;
        ortho_camera_desc.viewport_size = window::get_size();

        const mag::OrthographicCamera ortho_camera = mag::OrthographicCamera(ortho_camera_desc);

        auto light_entities = scene.get_ecs().get_all_components_of_types<TransformComponent, LightComponent>();

        // Draw physics colliders
        {
            struct Line
            {
                    vec3 position;
                    vec3 color;
            };

            std::vector<Line> lines;

            const mag::IPhysicsWorld* physics = scene.get_physics_world();

            const mag::math::LineList& line_list = physics->get_debug_line_list();

            if (line_list.lines.empty())
            {
                goto no_colliders;
            }

            mag::gfx::use_shader(shaders[LINE_SHADER]);

            struct GlobalData
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalData global_data = {};
            global_data.view = camera.get_view();
            global_data.projection = camera.get_projection();

            mag::gfx::set_uniform("u_global", &global_data);

            for (const mag::Line& line : line_list.lines)
            {
                lines.push_back({.position = line.start, .color = line.color});
                lines.push_back({.position = line.end, .color = line.color});
            }

            static mag::gfx::VertexBufferHandle vb = Max_U32;

            if (vb != Max_U32)
            {
                mag::gfx::destroy_vertex_buffer(vb);
            }

            vb = mag::gfx::create_vertex_buffer(VEC_SIZE_BYTES(lines), lines.data());

            mag::gfx::bind_vertex_buffer(vb);

            mag::gfx::draw(lines.size());
        }
    no_colliders:

        // Draw the floor
        {
            mag::gfx::use_shader(shaders[FLOOR_SHADER]);

            struct GlobalData
            {
                    mat4 view;
                    mat4 projection;
                    u32 light_count;
            };

            static GlobalData global_data = {};
            global_data.view = camera.get_view();
            global_data.projection = camera.get_projection();
            global_data.light_count = light_entities.size();

            // Lights buffer
            u32 light_num = 0;
            for (const auto& [transform, light] : light_entities)
            {
                LightData light_data = {};
                light_data.position = transform->translation;
                light_data.color = light->color;
                light_data.intensity = light->intensity;

                mag::gfx::set_uniform("u_light", &light_data, light_num++);
            }

            mag::gfx::set_uniform("u_global", &global_data);

            mag::gfx::draw(4);
        }

        // Draw the debug text
        {
            mag::gfx::use_shader(shaders[DEBUG_TEXT_SHADER]);

            // @TODO: temp
            struct FontData
            {
                    std::unordered_map<c8, mag::gfx::TextureHandle> char_texture_handles;
                    FontResource font;
                    u32 idx;
            };

            static std::unordered_map<str, FontData> fonts;

            struct GlobalData
            {
                    mat4 projection;
            };

            static GlobalData global_data = {};
            global_data.projection = ortho_camera.get_projection();

            mag::gfx::set_uniform("u_global", &global_data);

            TransformComponent transform;
            transform.scale = vec3(0.5f);
            transform.translation = vec3(-200.0f, -400.0f, 0.0f);

            const str font_name = "test_game/assets/fonts/FixedSys_Excelsior/FSEX300.ttf";

            // Quick way to calculate fps and frame time
            static f64 time = 0;
            static u64 frame_counter = 0;
            static u64 fps = 0;

            frame_counter++;
            time += dt;

            if (time >= 1.0)
            {
                fps = frame_counter;
                frame_counter = 0;
                time -= 1.0;
            }

            math::vec4 color = math::vec4(0.8f, 0.8f, 0.8f, 1.0f);

            if (fps > 100)
            {
                color = math::vec4(0.02f, 0.98f, 0.02f, 1.0f);
            }

            else if (fps >= 50 && fps <= 100)
            {
                color = math::vec4(0.98f, 0.98f, 0.02f, 1.0f);
            }

            else if (fps < 50)
            {
                color = math::vec4(0.98f, 0.02f, 0.02f, 1.0f);
            }

            const str text = mag::log::get_formatted_log("fps: {0}\ntime: {1:.3f} ms/frame", fps, dt * 1000.0);

            u32 char_offset = 0;

            if (!fonts.contains(font_name))
            {
                mag::FontResource font = {};
                ref<mag::FontResource> loaded_font = mag::resource::get_font(font_name);

                if (loaded_font != nullptr)
                {
                    font = *loaded_font;
                }

                FontData font_data = {};
                font_data.font = font;
                font_data.idx = fonts.size();  // The index is used to map a letter to the correct texture (and font)

                for (auto& [c, ch] : font.characters)
                {
                    // Skip non visual characters
                    if (ch.texture.pixels.empty())
                    {
                        continue;
                    }

                    font_data.char_texture_handles[c] =
                        mag::gfx::create_texture(ch.texture.width, ch.texture.height, ch.texture.pixels.size(),
                                                 ch.texture.pixels.data(), mag::gfx::Format::R8_UNORM);
                }

                fonts[font_name] = font_data;
            }

            const f32 scale = transform.scale.x;
            f32 x = transform.translation.x;
            f32 y = transform.translation.y;
            f32 z = transform.translation.z;

            for (const c8& c : text)
            {
                Character& ch = fonts[font_name].font.characters[c];

                // Skip chars with no visual representation (i.e. spaces)
                if (ch.data.empty())
                {
                    x += static_cast<f32>(ch.advance.x >> 6) * scale;
                    continue;
                }

                // Format newlines
                if (c == '\n')
                {
                    y -= static_cast<f32>(ch.size.y) * 1.5f * scale;  // @TODO: hardcoded line spacing
                    x = transform.translation.x;
                    continue;
                }

                // Don't offset the first letter of the text
                const f32 xpos = x + (char_offset > 0 ? static_cast<f32>(ch.bearing.x) * scale : 0);
                const f32 ypos = y - (char_offset > 0 ? static_cast<f32>(ch.size.y - ch.bearing.y) * scale : 0);
                const f32 zpos = z;

                TransformComponent char_transform;
                char_transform.translation = vec3(xpos, ypos, zpos);
                char_transform.scale =
                    vec3(static_cast<f32>(ch.size.x) * scale, static_cast<f32>(ch.size.y) * scale, 1.0f);
                char_transform.rotation = transform.rotation;

                // @TODO: rotation is a bit iffy but for now its ok
                const mat4 model_matrix = char_transform.get_transformation_matrix();

                const u32 font_offset = fonts[font_name].idx * 128;  // skip next 128 character textures
                const u32 texture_idx = font_offset + c;

                DebugTextData text_data = {};
                text_data.color = color;
                text_data.model = model_matrix;
                text_data.texture_idx = texture_idx;

                mag::gfx::set_uniform("u_instance", &text_data, char_offset);

                mag::gfx::set_uniform("u_char_textures", fonts[font_name].char_texture_handles[c], texture_idx);

                char_offset++;

                // Advance cursors for next glyph (note that advance is number of 1/64 pixels) bitshift by 6 to
                // get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
                x += static_cast<f32>(ch.advance.x >> 6) * scale;
            }

            mag::gfx::draw(4, char_offset);
        }
    }

    void Renderer::build_shader(const str& file_path, const b8 recompile)
    {
        if (recompile && !mag::resource::compile_shader(file_path))
        {
            LOG_ERROR("Failed to recompile shader: '{0}'", file_path);
            return;
        }

        ref<mag::ShaderResource> shader_resource = mag::resource::get_shader(file_path, recompile);
        MAG_ASSERT(shader_resource != nullptr, "Failed to load shader");

        // Destroy existing shader

        if (shaders.contains(file_path))
        {
            mag::gfx::destroy_shader(shaders[file_path]);
        }

        shaders[file_path] = mag::gfx::create_shader(*shader_resource);
    }
};  // namespace game
