#include "application.hpp"

#include <core/application.hpp>
#include <core/entry_point.hpp>
#include <core/event.hpp>
#include <project/project.hpp>
#include <resources/resource_loader.hpp>
#include <resources/shader.hpp>
#include <scene/scene.hpp>
#include <scene/scene_serializer.hpp>

mag::Application *mag::create_application() { return new game::TestGame("test_game/config.json"); }

namespace game
{
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
        mag::resource::load("magnolia/assets/shaders/triangle_shader.mag.json", &shader_resource);

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

        mat4 view = cam.get_view();
        mat4 proj = cam.get_projection();
        mat4 model = mag::math::scale(mat4(1.0f), mag::math::vec3(100.0f));
        mat4 view_proj[] = {view, proj, model};

        static mag::gfx::BufferHandle buf_handle = mag::gfx::create_buffer(sizeof(view_proj), view_proj);
        mag::gfx::set_buffer_data(buf_handle, sizeof(view_proj), view_proj);

        mag::gfx::use_shader(shader_handle);

        mag::gfx::set_shader_uniform(shader_handle, buf_handle);

        mag::gfx::draw(3);

        mag::gfx::end_frame();
    }

    void TestGame::on_event(const mag::Event &e) { scene->on_event(e); }
};  // namespace game
