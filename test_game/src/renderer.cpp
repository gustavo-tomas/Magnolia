#include "renderer.hpp"

#include <magnolia/ecs/ecs.hpp>
#include <magnolia/gfx/gfx.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/shader.hpp>

#include "components.hpp"
#include "scene.hpp"

// @TODO: temp
#include "../assets/shaders/include/common.h"

namespace game
{
    Renderer::Renderer()
    {
        // Load shaders

        mag::resource::compile_shader("test_game/assets/shaders/debug_text_shader.mag.json");

        ref<mag::ShaderResource> shader_resource = nullptr;

        shader_resource = mag::resource::get_shader("test_game/assets/shaders/debug_text_shader.mag.json");
        MAG_ASSERT(shader_resource != nullptr, "Failed to load shader");
        debug_text_shader = mag::gfx::create_shader(*shader_resource);
    }

    Renderer::~Renderer() = default;

    void Renderer::render_scene(Scene& scene, const f32 dt)
    {
        if (!mag::gfx::begin_frame())
        {
            return;
        }

        render_debug(scene, dt);

        if (!mag::gfx::end_frame())
        {
            return;
        }
    }

    void Renderer::on_event(const mag::Event& e) { mag::gfx::on_event(e); }

    void Renderer::render_debug(Scene& scene, const f32 dt)
    {
        OrhographicCameraDesc ortho_camera_desc = {};
        ortho_camera_desc.near = -100.0f;
        ortho_camera_desc.far = 100.0f;
        ortho_camera_desc.position = vec3(0.0f);
        ortho_camera_desc.rotation = vec3(0.0f);
        ortho_camera_desc.size = 1000.0f;
        ortho_camera_desc.viewport_size = window::get_size();

        const mag::OrthographicCamera ortho_camera = mag::OrthographicCamera(ortho_camera_desc);

        // Draw the debug text
        {
            mag::gfx::use_shader(debug_text_shader);

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
            transform.scale = vec3(1.0f);
            transform.translation = vec3(-300.0f, -200.0f, 0.0f);

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

            const str text = "Welcome to Magnolia!\nfps: " + std::to_string(fps) +
                             "\ntime: " + std::to_string(dt * 1000.0) + " ms/frame";

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

            for (auto& c : text)
            {
                Character& ch = fonts[font_name].font.characters[c];

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
                    x = transform.translation.x;
                    continue;
                }

                // Don't offset the first letter of the text
                const f32 xpos = x + (char_offset > 0 ? ch.bearing.x * scale : 0);
                const f32 ypos = y - (char_offset > 0 ? (ch.size.y - ch.bearing.y) * scale : 0);
                const f32 zpos = z;

                TransformComponent char_transform;
                char_transform.translation = vec3(xpos, ypos, zpos);
                char_transform.scale = vec3(ch.size.x * scale, ch.size.y * scale, 1.0f);
                char_transform.rotation = transform.rotation;

                // @TODO: rotation is a bit iffy but for now its ok
                const mat4 model_matrix = char_transform.get_transformation_matrix();

                const u32 font_offset = fonts[font_name].idx * 128;  // skip next 128 character textures
                const u32 texture_idx = font_offset + c;

                DebugTextData text_data;
                text_data.color = color;
                text_data.model = model_matrix;
                text_data.texture_idx = texture_idx;

                mag::gfx::set_uniform("u_instance", &text_data, char_offset);

                mag::gfx::set_uniform("u_char_textures", fonts[font_name].char_texture_handles[c], texture_idx);

                char_offset++;

                // Advance cursors for next glyph (note that advance is number of 1/64 pixels) bitshift by 6 to
                // get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
                x += (ch.advance.x >> 6) * scale;
            }

            mag::gfx::draw(4, char_offset);
        }
    }
};  // namespace game
