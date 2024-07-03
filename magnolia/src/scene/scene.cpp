#include "scene/scene.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "scene/scriptable_entity.hpp"

// @TODO: temp
#include "../sprout/assets/scripts/example_script.hpp"
#include "scripting/scripting_engine.hpp"

namespace mag
{
    Scene::Scene()
        : name("Untitled"),
          ecs(new ECS()),
          camera(new Camera({-200.0f, 50.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, 800.0f / 600.0f, 1.0f, 10000.0f)),
          camera_controller(new EditorCameraController(*camera)),
          render_pass(new StandardRenderPass({800, 600})),
          cube(new Cube())
    {
        auto& app = get_application();

        const auto& cube_model = cube->get_model();
        const auto& light_model = app.get_model_manager().load("magnolia/assets/models/lightbulb/lightbulb.glb");

        // Camera
        {
            const auto camera_entity = ecs->create_entity("Camera");

            auto* transform = new TransformComponent();
            auto* camera = new CameraComponent(new Camera(vec3(0), vec3(0), 60.0f, 800.0f / 600.0f, 1.0f, 10000.0f));
            auto* script = new ScriptComponent("sprout/assets/scripts/example_lua_script.lua");
            // auto* script = new NativeScriptComponent();
            // script->bind<CameraController>();

            ecs->add_component(camera_entity, transform);
            ecs->add_component(camera_entity, camera);
            ecs->add_component(camera_entity, script);
        }

        const i32 loops = 2;
        u32 count = 0;
        for (i32 i = -loops / 2; i <= loops / 2; i++)
        {
            for (i32 j = -loops / 2; j <= loops / 2; j++)
            {
                const str name = "Cube" + std::to_string(count++);

                const auto cube_entity = ecs->create_entity(name);

                const vec3 scale = vec3(10);
                const vec3 rot = vec3(rand() % 360);
                const vec3 pos = vec3(i * 30, 100, j * 30);

                auto* transform = new TransformComponent(pos, rot, scale);
                auto* rigid_body = new RigidBodyComponent(1.0f);
                auto* collider = new BoxColliderComponent(transform->scale);

                ecs->add_component(cube_entity, transform);
                ecs->add_component(cube_entity, new ModelComponent(cube_model));
                ecs->add_component(cube_entity, collider);
                ecs->add_component(cube_entity, rigid_body);

                render_pass->add_model(*ecs, cube_entity);

                LOG_INFO("Cube created");
            }
        }

        // Floor
        {
            const auto floor_entity = ecs->create_entity("Floor");

            const vec3 pos = vec3(0, -10, 0);

            auto* transform = new TransformComponent(pos);
            auto* rigid_body = new RigidBodyComponent(0.0f);
            auto* collider = new BoxColliderComponent(vec3(150, 10, 150));

            ecs->add_component(floor_entity, transform);
            ecs->add_component(floor_entity, collider);
            ecs->add_component(floor_entity, rigid_body);
        }

        // Light
        for (i32 i = 0; i < static_cast<i32>(LightComponent::MAX_NUMBER_OF_LIGHTS); i++)
        {
            const auto light = ecs->create_entity("Light" + std::to_string(i));
            const vec3 color = vec3(1);
            const f32 intensity = 100;

            ecs->add_component(light, new LightComponent(color, intensity));
            ecs->add_component(light, new ModelComponent(*light_model));
            ecs->add_component(light, new TransformComponent(vec3(500 - (500 * i), 100, 0), vec3(0), vec3(10)));

            render_pass->add_model(*ecs, light);

            LOG_INFO("Light created");
        }
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

        runtime_ecs = std::make_unique<ECS>(*ecs);
        current_state = SceneState::Runtime;

        physics_engine.on_simulation_start();

        ScriptingEngine::new_state();

        // Instanciate the script
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
        auto& app = get_application();
        auto& window = app.get_window();

        ecs->update();

        camera_controller->update(dt);

        // @TODO: testing
        if (window.is_key_down(Key::Up))
            render_pass->set_render_scale(render_pass->get_render_scale() + 0.15f * dt);

        else if (window.is_key_down(Key::Down))
            render_pass->set_render_scale(render_pass->get_render_scale() - 0.15f * dt);
        // @TODO: testing
    }

    void Scene::update_runtime(const f32 dt)
    {
        runtime_ecs->update();

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
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<ViewportResizeEvent>(BIND_FN(Scene::on_viewport_resize));

        camera_controller->on_event(e);
    }

    void Scene::on_viewport_resize(ViewportResizeEvent& e)
    {
        const uvec2 size = {e.width, e.height};

        render_pass->on_resize(size);
        camera->set_aspect_ratio(size);
    };

    void Scene::add_model(const str& path)
    {
        auto& app = get_application();
        auto& model_manager = app.get_model_manager();

        const auto model = model_manager.load(path);

        const auto entity = ecs->create_entity();
        ecs->add_component(entity, new TransformComponent());
        ecs->add_component(entity, new ModelComponent(*model));

        render_pass->add_model(*ecs, entity);
    }

    void Scene::remove_model(const u32 id)
    {
        ecs->add_entity_to_deletion_queue(id);

        render_pass->remove_model(*ecs, id);
    }
};  // namespace mag
