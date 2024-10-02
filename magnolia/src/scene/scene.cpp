#include "scene/scene.hpp"

#include "core/application.hpp"
#include "renderer/test_model.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/scripting_engine.hpp"

// @TODO: 'remove' viewport stuff
// Editor specific stuff should be decoupled from the Scene/Application

namespace mag
{
    Scene::Scene()
        : name("Untitled"),
          ecs(new ECS()),
          camera(new Camera({0.0f, 50.0f, 150.0f}, vec3(0.0f), 60.0f, 800.0f / 600.0f, 1.0f, 10000.0f)),
          camera_controller(new EditorCameraController(*camera))
    {
        auto& app = get_application();
        auto& window = app.get_window();

        const uvec2 window_size = window.get_size();
        current_viewport_size = window_size;

        camera->set_aspect_ratio(current_viewport_size);
    }

    Scene::~Scene()
    {
        if (runtime_ecs)
        {
            stop_runtime();
        }
    }

    void Scene::update(const f32 dt)
    {
        switch (current_state)
        {
            case SceneState::Editor:
                update_editor(dt);
                break;

            case SceneState::Runtime:
                update_runtime(dt);
                break;

            default:
                break;
        }
    }

    void Scene::start_runtime()
    {
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        runtime_ecs = create_unique<ECS>(*ecs);
        current_state = SceneState::Runtime;

        physics_engine.on_simulation_start(this);

        ScriptingEngine::new_state();

        // Instantiate the script
        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* sc = runtime_ecs->get_component<ScriptComponent>(id);
            if (!sc->instance)
            {
                sc->instance = new Script();
                sc->instance->ecs = runtime_ecs.get();
                sc->instance->entity_id = id;

                ScriptingEngine::load_script(sc->file_path);
                ScriptingEngine::register_entity(*sc);

                sc->instance->on_create(*sc->instance);
            }
        }

        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<NativeScriptComponent>())
        {
            auto* nsc = runtime_ecs->get_component<NativeScriptComponent>(id);
            if (!nsc->instance)
            {
                nsc->instance = nsc->instanciate_script();
                nsc->instance->ecs = runtime_ecs.get();
                nsc->instance->entity_id = id;
                nsc->instance->on_create();
            }
        }
    }

    void Scene::stop_runtime()
    {
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = runtime_ecs->get_component<ScriptComponent>(id);
            script->instance->on_destroy(*script->instance);
            delete script->instance;
            script->instance = nullptr;
        }

        for (auto nsc : runtime_ecs->get_all_components_of_type<NativeScriptComponent>())
        {
            nsc->instance->on_destroy();
            nsc->destroy_script(nsc);
        }

        runtime_ecs.reset();
        current_state = SceneState::Editor;

        physics_engine.on_simulation_end();
    }

    void Scene::update_editor(const f32 dt)
    {
        // Delete enqueued entities
        for (const auto& entity : entity_deletion_queue)
        {
            ecs->erase_entity(entity);
        }

        entity_deletion_queue.clear();

        camera_controller->update(dt);

        // Update camera transforms
        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            camera_c->camera.set_position(transform->translation);
            camera_c->camera.set_rotation(transform->rotation);
        }
    }

    void Scene::update_runtime(const f32 dt)
    {
        // Set camera positions the same as the transform
        // @TODO: there should be only one value for the camera position. I feel that data duplication
        // could cause issues in the future.

        auto components = runtime_ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            camera_c->camera.set_position(transform->translation);
            camera_c->camera.set_rotation(transform->rotation);
        }

        // Update scripts
        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = runtime_ecs->get_component<ScriptComponent>(id);
            script->instance->on_update(*script->instance, dt);
        }

        for (auto nsc : runtime_ecs->get_all_components_of_type<NativeScriptComponent>())
        {
            nsc->instance->on_update(dt);
        }
    }

    void Scene::on_event(Event& e)
    {
        // Don't process editor input during runtime
        if (current_state == SceneState::Editor)
        {
            camera_controller->on_event(e);
        }
    }

    void Scene::on_viewport_resize(const uvec2& new_viewport_size)
    {
        if (new_viewport_size == current_viewport_size) return;

        current_viewport_size = new_viewport_size;

        camera->set_aspect_ratio(current_viewport_size);

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(current_viewport_size);
        }

        if (!runtime_ecs) return;

        for (auto camera_c : runtime_ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(current_viewport_size);
        }
    }

    // void Scene::set_render_scale(const f32 new_render_scale)
    // {
    //     (void)
    //     // render_scale = std::clamp(new_render_scale, 0.01f, 1.0f);

    //     // auto e = WindowResizeEvent(draw_size.x, draw_size.y);
    //     // on_resize(e);
    //     // // build_render_graph(draw_size);
    // }

    void Scene::add_model(const str& path)
    {
        auto& app = get_application();
        auto& model_manager = app.get_model_manager();

        const auto model = model_manager.get(path);

        const auto entity = ecs->create_entity();
        ecs->add_component(entity, new TransformComponent());
        ecs->add_component(entity, new ModelComponent(model));
    }

    void Scene::add_sprite(const str& path)
    {
        auto& app = get_application();
        auto& texture_manager = app.get_texture_manager();

        const auto sprite = texture_manager.get(path);

        const auto entity = ecs->create_entity();
        ecs->add_component(entity, new SpriteComponent(sprite, path));
        ecs->add_component(entity, new TransformComponent());
    }

    void Scene::remove_entity(const u32 id)
    {
        if (!ecs->entity_exists(id)) return;

        // Enqueue entity
        entity_deletion_queue.push_back(id);
    }
};  // namespace mag
