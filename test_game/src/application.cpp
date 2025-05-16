#include "application.hpp"

#include <core/application.hpp>
#include <core/entry_point.hpp>
#include <core/event.hpp>
#include <gfx/gfx.hpp>
#include <project/project.hpp>
#include <resources/image.hpp>
#include <resources/resource_loader.hpp>
#include <resources/shader.hpp>
#include <scene/scene.hpp>
#include <scene/scene_serializer.hpp>

// @TODO: temp
#include "../magnolia/assets/shaders/include/common.h"

mag::Application *mag::create_application() { return new game::TestGame("test_game/config.json"); }

namespace game
{
    // @TODO: temp
    static std::vector<mag::gfx::TextureHandle> texture_handles;
    static std::vector<mag::Image> textures;

    TestGame::TestGame(const str &config_file_path) : Application(config_file_path)
    {
        // Load the project

        mag::Project project;

        const str project_file_path = "test_game/TestGame.proj.json";
        if (!mag::project::load(project_file_path, project))
        {
            LOG_ERROR("Failed to load project: '{0}'", project_file_path);
            return;
        }

        // @TODO: temp - load shaders

        mag::ShaderResource shader_resource = {};
        mag::resource::load("magnolia/assets/shaders/sprite_shader.mag.json", &shader_resource);

        shader_handle = mag::gfx::create_shader(shader_resource);

        // Then load starting scene

        scene = create_unique<mag::Scene>();

        const str start_scene_file_path = project.get_asset_dir() / project.get_relative_start_scene_path();
        if (!mag::scene::load(start_scene_file_path, *scene))
        {
            LOG_ERROR("Failed to load start scene: '{0}'", start_scene_file_path);
            return;
        }

        scene->on_start();

        u32 texture_count = 2;

        textures.resize(texture_count);

        for (u32 i = 0; i < texture_count; i++)
        {
            MAG_ASSERT(mag::resource::load("magnolia/assets/test_texture" + std::to_string(i) + ".png", &textures[i]),
                       "Failed to load textures[{0}]", i);
            texture_handles.push_back(mag::gfx::create_texture(textures[i].width, textures[i].height,
                                                               textures[i].pixels.size(), textures[i].pixels.data()));
        }
    }

    TestGame::~TestGame() = default;

    void TestGame::on_update(const f32 dt)
    {
        // Check for scene swaps
        const str &next_scene_file_path = scene->get_next_scene();
        if (!next_scene_file_path.empty())
        {
            mag::Scene *next_scene = new mag::Scene();
            if (!mag::scene::load(next_scene_file_path, *next_scene))
            {
                LOG_ERROR("Failed to load scene: '{0}'", next_scene_file_path);
                delete next_scene;
            }

            else
            {
                scene.reset(next_scene);
                scene->on_start();
            }

            scene->set_next_scene("");
        }

        scene->on_update(dt);

        // Render the triangle

        mag::Camera &cam = scene->get_camera();

        mag::gfx::begin_frame();

        mag::gfx::use_shader(shader_handle);

        // Global buffer
        {
            struct alignas(16) GlobalBuffer
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalBuffer global_buffer = {};

            global_buffer.view = cam.get_view();
            global_buffer.projection = cam.get_projection();

            static mag::gfx::BufferHandle buf_handle = mag::gfx::create_buffer(sizeof(GlobalBuffer), &global_buffer);
            mag::gfx::set_buffer_data(buf_handle, &global_buffer, sizeof(GlobalBuffer));
            mag::gfx::set_shader_buffer_uniform(shader_handle, buf_handle, 0);
        }

        for (u32 i = 0; i < texture_handles.size(); i++)
        {
            static SpriteData sprite_data = {};
            sprite_data.model = math::translate(mat4(1.0f), vec3(i * 100.0f, 0, 0));
            sprite_data.size_const_face = vec4(textures[i].width * 0.05f, textures[i].height * 0.05f, 0, 0);
            sprite_data.texture_idx = i;

            // Instance buffer
            static mag::gfx::BufferHandle buf_handle_sprite = mag::gfx::create_buffer(sizeof(SpriteData) * 2);
            mag::gfx::set_buffer_data(buf_handle_sprite, &sprite_data, sizeof(SpriteData), sizeof(SpriteData) * i);
            mag::gfx::set_shader_buffer_uniform(shader_handle, buf_handle_sprite, 1, i);

            // Textures
            mag::gfx::set_shader_texture_uniform(shader_handle, texture_handles[i], 2, i);

            mag::gfx::draw(4, 1, 0, i);
        }

        mag::gfx::end_frame();
    }

    void TestGame::on_event(const mag::Event &e) { scene->on_event(e); }
};  // namespace game
