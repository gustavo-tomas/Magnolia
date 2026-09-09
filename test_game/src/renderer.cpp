#include "renderer.hpp"

#include <magnolia/core/logger.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/gfx/gfx.hpp>
#include <magnolia/math/functions.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/file_system.hpp>
#include <magnolia/platform/platform.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/material.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/shader.hpp>
#include <magnolia/resources/texture.hpp>

#include "ecs/components.hpp"
#include "scene.hpp"

// @TODO: temp
#include "../assets/shaders/include/common.h"

// This is the game renderer. It sits one layer above the gfx frontend and manages shaders, buffers and textures.

namespace game
{
#define MESH_SHADER "test_game/assets/shaders/mesh_shader.mag.json"
#define SPRITE_SHADER "test_game/assets/shaders/sprite_shader.mag.json"
#define TEXT_SHADER "test_game/assets/shaders/text_shader.mag.json"
#define GRASS_SHADER "test_game/assets/shaders/grass_shader.mag.json"

#if MAG_CONFIG_DEBUG
    #define FLOOR_SHADER "test_game/assets/shaders/floor_shader.mag.json"
    #define LINE_SHADER "test_game/assets/shaders/line_shader.mag.json"
    #define DEBUG_TEXT_SHADER "test_game/assets/shaders/debug_text_shader.mag.json"
#endif

    void Renderer::initialize()
    {
        // Load shaders

        build_shader(MESH_SHADER, false);
        build_shader(SPRITE_SHADER, false);
        build_shader(TEXT_SHADER, false);
        build_shader(GRASS_SHADER, false);

#if MAG_CONFIG_DEBUG
        initialize_debug_system();
#endif
    }

    void Renderer::shutdown()
    {
#if MAG_CONFIG_DEBUG
        shutdown_debug_system();
#endif

        for (const auto& [name, handle] : vertex_buffer_handles)
        {
            mag::gfx::destroy_vertex_buffer(handle);
        }

        for (const auto& [name, handle] : index_buffer_handles)
        {
            mag::gfx::destroy_index_buffer(handle);
        }

        for (const auto& [name, shader] : shaders)
        {
            mag::gfx::destroy_shader(shader);
        }
    }

    void Renderer::render_scene(Scene& scene, const f32 dt)
    {
        if (!mag::gfx::begin_frame())
        {
            return;
        }

        render_sprites(scene);
        render_models(scene);
        // render_grass(scene);
        render_text(scene);
#if MAG_CONFIG_DEBUG
        render_debug(scene, dt);
#endif

        if (!mag::gfx::end_frame())
        {
            return;
        }
    }

    void Renderer::render_models(Scene& scene)
    {
        auto& ecs = scene.get_ecs();
        auto& camera = scene.get_camera();

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();

        if (model_entities.empty())
        {
            return;
        }

        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();

        // Render models

        mag::gfx::use_shader(shaders[MESH_SHADER]);

        // Global buffer
        struct GlobalData
        {
                CameraData camera = {};
                u32 light_count = 0;
        };

        GlobalData global_data = {};
        global_data.camera.view = camera.get_view();
        global_data.camera.projection = camera.get_projection();
        global_data.light_count = light_entities.size();

        mag::gfx::set_uniform("u_global", &global_data);

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

        u32 model_offset = 0;
        u32 mesh_offset = 0;
        u32 material_offset = 0;
        u32 texture_offset = 0;

        for (auto& model_entity : model_entities)
        {
            const auto& transform = std::get<0>(model_entity);
            const auto& model = std::get<1>(model_entity)->model;
            const str& model_name = model->file_path;

            mag::gfx::bind_vertex_buffer(vertex_buffer_handles[model_name]);
            mag::gfx::bind_index_buffer(index_buffer_handles[model_name]);

            ModelData model_data = {};
            model_data.model = transform->get_transformation_matrix();

            mag::gfx::set_uniform("u_model", &model_data, model_offset);

            u32 mesh_materials_bound = 0;
            u32 last_bound_material_idx = Max_U32;
            for (auto& mesh : model->meshes)
            {
                // Instance
                MeshData mesh_data = {};
                mesh_data.model_idx = model_offset;
                mesh_data.material_idx = mesh.material_index + material_offset;

                // Set the material. The meshes are sorted by material index (see model loader), so we draw all
                // meshes with the same material before swapping to the next one.
                if (last_bound_material_idx != mesh.material_index)
                {
                    last_bound_material_idx = mesh.material_index;
                    mesh_data.material_idx = mesh.material_index + material_offset;

                    const ref<mag::MaterialResource>& material = model->materials[mesh.material_index];

                    // @TODO: hardcoded material parameters
                    MaterialData material_data = {};
                    material_data.albedo = vec4(1, 1, 1, 1);
                    material_data.roughness = 1;
                    material_data.metallic = 1;
                    material_data.albedo_tex_idx = texture_offset;
                    material_data.normal_tex_idx = texture_offset + 1;
                    material_data.roughness_tex_idx = texture_offset + 2;
                    material_data.metalness_tex_idx = texture_offset + 3;

                    mag::gfx::set_uniform("u_material", &material_data, mesh_data.material_idx);

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
                                          texture_offset + 3);  // ARM texture

                    mesh_materials_bound++;
                    texture_offset += 4;
                }

                mag::gfx::set_uniform("u_instance", &mesh_data, mesh_offset);

                // Draw the mesh
                mag::gfx::draw_indexed(mesh.index_count, 1, mesh.base_index, static_cast<i32>(mesh.base_vertex),
                                       mesh_offset);

                mesh_offset++;
            }

            material_offset += mesh_materials_bound;
            model_offset++;
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

        CameraData global_data = {};
        global_data.view = camera.get_view();
        global_data.projection = camera.get_projection();

        mag::gfx::set_uniform("u_global", &global_data);

        u32 texture_offset = 0;
        for (auto& sprite_entity : sprite_entities)
        {
            const auto& transform = std::get<0>(sprite_entity);
            const auto& sprite = std::get<1>(sprite_entity);
            const str& name = sprite->texture->file_path;

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

    // Grass
    static b8 init = false;

    const i32 count = 300;
    const f32 patch_spread = 1.0F;
    const f32 position_variation = 0.7F;

    void Renderer::set_grass_uniforms()
    {
        u32 instance = 0;
        for (i32 i = -count / 2; i < count / 2; i++)
        {
            for (i32 j = -count / 2; j < count / 2; j++)
            {
                const vec3 position = vec3(
                    (static_cast<f32>(i) * patch_spread) + math::random(-position_variation, position_variation), 0,
                    (static_cast<f32>(j - (count / 2) - 10) * patch_spread) +
                        math::random(-position_variation, position_variation));

                GrassData grass_data = {};
                grass_data.position = position;

                mag::gfx::set_uniform_static("u_instance", &grass_data, instance++);
            }
        }
    }

    // @TODO: don't use random values. Use a texture/noise function instead so the results stay consistent.
    void Renderer::render_grass(Scene& scene)
    {
        mag::gfx::use_shader(shaders[GRASS_SHADER]);

        struct GrassVertex
        {
                vec3 position;
                vec3 normal;
        };

        static std::vector<GrassVertex> grass_vertices;

        static std::vector<u32> grass_indices;

        static f32 max_height = 0.0F;

        if (!init)
        {
            ref<ModelResource> grass_model;
            grass_model = mag::resource::get_model("test_game/assets/models/grass/native/Grass.001.model.json");

            // Quick hack to get max blade height
            max_height = grass_model->meshes[0].aabb_max.y;

            // Apply scale directly to the vertex
            const vec3 scale = vec3(3.0F);
            const mat4 model_matrix = math::scale(mat4(1.0F), scale);

            for (Vertex& v : grass_model->vertices)
            {
                GrassVertex grass_vertex = {};
                grass_vertex.position = vec3(model_matrix * vec4(v.position, 1.0F));
                grass_vertex.normal = v.normal;

                grass_vertices.push_back(grass_vertex);
            }

            grass_indices = grass_model->indices;

            const mag::gfx::VertexBufferHandle vertex_buffer =
                mag::gfx::create_vertex_buffer(VEC_SIZE_BYTES(grass_vertices), grass_vertices.data());

            const mag::gfx::VertexBufferHandle index_buffer =
                mag::gfx::create_index_buffer(VEC_SIZE_BYTES(grass_indices), grass_indices.data());

            vertex_buffer_handles[GRASS_SHADER] = vertex_buffer;
            index_buffer_handles[GRASS_SHADER] = index_buffer;

            init = true;
        }

        if (grass_uniforms_need_update)
        {
            // Uniforms only need to be set once
            set_grass_uniforms();

            grass_uniforms_need_update = false;
        }

        mag::Camera& camera = scene.get_camera();
        auto light_entities = scene.get_ecs().get_all_components_of_types<TransformComponent, LightComponent>();

        GlobalGrassData global_data = {};
        global_data.camera_data.view = camera.get_view();
        global_data.camera_data.projection = camera.get_projection();
        global_data.light_count = light_entities.size();
        global_data.time = static_cast<f32>(mag::plat::get_time());
        global_data.max_blade_height = max_height;

        mag::gfx::set_uniform("u_global", &global_data);

        mag::gfx::bind_vertex_buffer(vertex_buffer_handles[GRASS_SHADER]);
        mag::gfx::bind_index_buffer(index_buffer_handles[GRASS_SHADER]);

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

        mag::gfx::draw_indexed(grass_indices.size(), count * count);
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

        CameraData global_data = {};
        global_data.view = camera.get_view();
        global_data.projection = camera.get_projection();

        mag::gfx::set_uniform("u_global", &global_data);

        u32 char_offset = 0;
        for (auto& text_entity : text_entities)
        {
            const auto& transform = std::get<0>(text_entity);
            const auto& text = std::get<1>(text_entity);
            const str& name = text->font->file_path;

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
                    y -= static_cast<f32>(ch.size.y) * 1.5F * scale;  // @TODO: hardcoded line spacing
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
                    vec3(static_cast<f32>(ch.size.x) * scale, static_cast<f32>(ch.size.y) * scale, 1.0F);
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

#if MAG_CONFIG_DEBUG
    const u64 max_buffer_size = 64ULL * 1024 * 1024;
    const str debug_font_name = "test_game/assets/fonts/FixedSys_Excelsior/FSEX300.ttf";

    void Renderer::initialize_debug_system()
    {
        vb = mag::gfx::create_vertex_buffer(max_buffer_size, nullptr);

        build_shader(LINE_SHADER, false);
        build_shader(FLOOR_SHADER, false);
        build_shader(DEBUG_TEXT_SHADER, false);

        debug_font = mag::resource::get_font(debug_font_name);

        if (debug_font != nullptr)
        {
            on_font_added(*debug_font);
        }
    }

    void Renderer::shutdown_debug_system() const { mag::gfx::destroy_vertex_buffer(vb); }

    void Renderer::draw_debug_colliders(Scene& scene, const f32 dt)
    {
        (void)dt;

        mag::Camera& camera = scene.get_camera();

        struct Line
        {
                vec3 position;
                vec3 color;
        };

        const mag::physics::IPhysicsWorld& physics = scene.get_physics_world();

        const mag::math::LineList& line_list = physics.get_debug_line_list();

        if (line_list.lines.empty())
        {
            return;
        }

        mag::gfx::use_shader(shaders[LINE_SHADER]);

        struct GlobalData
        {
                mat4 view;
                mat4 projection;
        };

        GlobalData global_data = {};
        global_data.view = camera.get_view();
        global_data.projection = camera.get_projection();

        mag::gfx::set_uniform("u_global", &global_data);

        std::vector<Line> lines(line_list.lines.size());

        for (u64 i = 0; i < line_list.lines.size(); i++)
        {
            const math::Line& line = line_list.lines[i];
            lines[i] = {.position = line.start, .color = line.color};
            lines[i] = {.position = line.end, .color = line.color};
        }

        const u64 line_vec_size = VEC_SIZE_BYTES(lines);

        if (line_vec_size > max_buffer_size)
        {
            LOG_ERROR("Debug buffer size exceeded ('{0}' < '{1}'). Please increase buffer limit.", max_buffer_size,
                      line_vec_size);
            return;
        }

        mag::gfx::set_buffer_data(vb, lines.data(), line_vec_size, 0);

        mag::gfx::bind_vertex_buffer(vb);

        mag::gfx::draw(lines.size());
    }

    void Renderer::draw_debug_floor(Scene& scene, const f32 dt)
    {
        (void)dt;

        mag::ECS& ecs = scene.get_ecs();
        mag::Camera& camera = scene.get_camera();

        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();

        mag::gfx::use_shader(shaders[FLOOR_SHADER]);

        struct GlobalData
        {
                mat4 view = mat4(1.0F);
                mat4 projection = mat4(1.0F);
                u32 light_count = 0;
        };

        GlobalData global_data = {};
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

    void Renderer::draw_debug_text(Scene& scene, const f32 dt)
    {
        (void)scene;

        const OrthographicCameraDesc ortho_camera_desc = {
            .position = vec3(0.0F),
            .rotation = quat(vec3(0.0F)),
            .viewport_size = vec2(window::get_size()),
            .near = -100.0F,
            .far = 100.0F,
            .size = 1000.0F,
        };

        mag::OrthographicCamera ortho_camera = mag::OrthographicCamera(ortho_camera_desc);

        mag::gfx::use_shader(shaders[DEBUG_TEXT_SHADER]);

        frame_counter++;
        time += dt;

        if (time >= 1.0)
        {
            fps = frame_counter;
            frame_counter = 0;
            time -= 1.0;
        }

        math::vec3 color = math::vec3(0.8F, 0.8F, 0.8F);

        if (fps > 100)
        {
            color = math::vec3(0.02F, 0.98F, 0.02F);
        }

        else if (fps >= 50 && fps <= 100)
        {
            color = math::vec3(0.98F, 0.98F, 0.02F);
        }

        else if (fps < 50)
        {
            color = math::vec3(0.98F, 0.02F, 0.02F);
        }

        struct GlobalData
        {
                mat4 projection;
                vec3 color;
        };

        const GlobalData global_data = {
            .projection = ortho_camera.get_projection(),
            .color = color,
        };

        mag::gfx::set_uniform("u_global", &global_data);

        const str text = mag::log::get_formatted_log("fps: {0}\ntime: {1:.3f} ms/frame", fps, dt * 1000.0);

        u32 char_offset = 0;

        const f32 scale = 0.5F;
        const vec2 base_position(-200.0F, -400.0F);
        f32 x = base_position.x;
        f32 y = base_position.y;

        const FontData& font_data = fonts[debug_font_name];

        for (const c8 c : text)
        {
            const Character& ch = debug_font->characters[c];

            // Skip chars with no visual representation (i.e. spaces)
            if (ch.data.empty())
            {
                x += static_cast<f32>(ch.advance.x >> 6) * scale;
                continue;
            }

            // Format newlines
            if (c == '\n')
            {
                y -= static_cast<f32>(ch.size.y) * 1.5F * scale;
                x = base_position.x;
                continue;
            }

            // Don't offset the first letter of the text
            const f32 xpos = x + (char_offset > 0 ? static_cast<f32>(ch.bearing.x) * scale : 0);
            const f32 ypos = y - (char_offset > 0 ? static_cast<f32>(ch.size.y - ch.bearing.y) * scale : 0);

            const DebugTextData text_data = {
                .position = vec2(xpos, ypos),
                .scale = vec2(static_cast<f32>(ch.size.x) * scale, static_cast<f32>(ch.size.y) * scale),
                .texture_idx = static_cast<u32>(c),
            };

            mag::gfx::set_uniform("u_instance", &text_data, char_offset);

            mag::gfx::set_uniform("u_char_textures", font_data.char_texture_handles.at(c), c);

            char_offset++;

            // Advance cursors for next glyph (note that advance is number of 1/64 pixels) bitshift by 6 to
            // get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            x += static_cast<f32>(ch.advance.x >> 6) * scale;
        }

        mag::gfx::draw(4, char_offset);
    }

    void Renderer::render_debug(Scene& scene, const f32 dt)
    {
        draw_debug_colliders(scene, dt);
        draw_debug_floor(scene, dt);
        draw_debug_text(scene, dt);
    }
#endif

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

        // Set uniforms again
        if (file_path == GRASS_SHADER && init)
        {
            grass_uniforms_need_update = true;
        }
    }

    void Renderer::on_event(const mag::Event& e) { mag::gfx::on_event(e); }

    void Renderer::on_model_added(const mag::ModelResource& model)
    {
        const str& name = model.file_path;

        if (model.loading_status != LoadingStatus::Finished)
        {
            LOG_WARNING("Model '{0}' has not finished loading", name);
        }

        if (vertex_buffer_handles.contains(name))
        {
            return;
        }

        const mag::gfx::VertexBufferHandle vertex_buffer =
            mag::gfx::create_vertex_buffer(VEC_SIZE_BYTES(model.vertices), model.vertices.data());

        const mag::gfx::IndexBufferHandle index_buffer =
            mag::gfx::create_index_buffer(VEC_SIZE_BYTES(model.indices), model.indices.data());

        vertex_buffer_handles[name] = vertex_buffer;
        index_buffer_handles[name] = index_buffer;

        for (const auto& material : model.materials)
        {
            for (const auto& [slot, texture] : material->textures)
            {
                on_texture_added(*texture);
            }
        }
    }

    void Renderer::on_texture_added(const mag::TextureResource& texture)
    {
        const str& name = texture.file_path;

        if (texture.loading_status != LoadingStatus::Finished)
        {
            LOG_WARNING("Texture '{0}' has not finished loading", name);
        }

        if (texture_handles.contains(name))
        {
            return;
        }

        texture_handles[name] =
            mag::gfx::create_texture(texture.width, texture.height, texture.pixels.size(), texture.pixels.data());
    }

    void Renderer::on_font_added(const mag::FontResource& font)
    {
        const str& name = font.file_path;

        if (font.loading_status != LoadingStatus::Finished)
        {
            LOG_WARNING("Font '{0}' has not finished loading", name);
        }

        if (fonts.contains(name))
        {
            return;
        }

        FontData font_data = {};
        font_data.idx = fonts.size();  // The index is used to map a letter to the correct texture (and font)

        for (const auto& [c, ch] : font.characters)
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
};  // namespace game
