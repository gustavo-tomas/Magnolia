#include <magnolia/core/logger.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/gfx/gfx.hpp>
#include <magnolia/gfx/types.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/shader.hpp>
#include <magnolia/resources/texture.hpp>

#include "../../assets/shaders/include/common.h"
#include "components.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "systems.hpp"

namespace game
{
#define FLOOR_SHADER "test_game/assets/shaders/floor_shader.mag.json"
#define LINE_SHADER "test_game/assets/shaders/line_shader.mag.json"
#define DEBUG_TEXT_SHADER "test_game/assets/shaders/debug_text_shader.mag.json"

    static std::unordered_map<str, mag::gfx::ShaderHandle> shaders;

    static void build_shader(const str& file_path, const b8 recompile)
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

    static void draw_colliders(Scene& scene, const f32 dt)
    {
        (void)dt;

        mag::Camera& camera = scene.get_camera();

        struct Line
        {
                vec3 position;
                vec3 color;
        };

        std::vector<Line> lines;

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

        static GlobalData global_data = {};
        global_data.view = camera.get_view();
        global_data.projection = camera.get_projection();

        mag::gfx::set_uniform("u_global", &global_data);

        for (const mag::math::Line& line : line_list.lines)
        {
            lines.push_back({.position = line.start, .color = line.color});
            lines.push_back({.position = line.end, .color = line.color});
        }

        static mag::gfx::VertexBufferHandle vb = mag::Invalid_ID;

        if (vb != mag::Invalid_ID)
        {
            mag::gfx::destroy_vertex_buffer(vb);
        }

        vb = mag::gfx::create_vertex_buffer(VEC_SIZE_BYTES(lines), lines.data());

        mag::gfx::bind_vertex_buffer(vb);

        mag::gfx::draw(lines.size());
    }

    static void draw_floor(Scene& scene, const f32 dt)
    {
        (void)dt;

        mag::ECS& ecs = scene.get_ecs();
        mag::Camera& camera = scene.get_camera();

        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();

        mag::gfx::use_shader(shaders[FLOOR_SHADER]);

        struct GlobalData
        {
                mat4 view;
                mat4 projection;
                u32 light_count;
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

    static void draw_text(Scene& scene, const f32 dt)
    {
        (void)scene;

        OrthographicCameraDesc ortho_camera_desc = {};
        ortho_camera_desc.near = -100.0f;
        ortho_camera_desc.far = 100.0f;
        ortho_camera_desc.position = vec3(0.0f);
        ortho_camera_desc.rotation = quat(vec3(0.0f));
        ortho_camera_desc.size = 1000.0f;
        ortho_camera_desc.viewport_size = vec2(window::get_size());

        mag::OrthographicCamera ortho_camera = mag::OrthographicCamera(ortho_camera_desc);

        mag::gfx::use_shader(shaders[DEBUG_TEXT_SHADER]);

        // @TODO: temp
        struct FontData
        {
                std::unordered_map<c8, mag::gfx::TextureHandle> char_texture_handles;
                mag::FontResource font;
                u32 idx;
        };

        static std::unordered_map<str, FontData> fonts;

        struct GlobalData
        {
                mat4 projection;
        };

        GlobalData global_data = {};
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
            char_transform.scale = vec3(static_cast<f32>(ch.size.x) * scale, static_cast<f32>(ch.size.y) * scale, 1.0f);
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

    void debug_system(Scene& scene, const f32 dt)
    {
        static b8 init = false;

        if (!init)
        {
            build_shader(LINE_SHADER, false);
            build_shader(FLOOR_SHADER, false);
            build_shader(DEBUG_TEXT_SHADER, false);

            init = true;
        }

        draw_colliders(scene, dt);
        draw_floor(scene, dt);
        draw_text(scene, dt);
    }
};  // namespace game
