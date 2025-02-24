#include "scene/scene.hpp"

#include "camera/camera.hpp"
#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "ecs/components.hpp"
#include "math/generic.hpp"
#include "physics/physics.hpp"
#include "renderer/test_model.hpp"
#include "resources/image.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/scripting_engine.hpp"

namespace mag
{
    Scene::Scene()
        : name("Untitled"), ecs(new ECS(10'000, BIND_FN2(Scene::on_component_added))), physics_world(new PhysicsWorld())
    {
    }

    Scene::~Scene()
    {
        if (running)
        {
            on_stop();
        }
    }

    void Scene::on_start()
    {
        on_start_internal();

        instantiate_scripts();

        running = true;
    }

    void Scene::instantiate_scripts()
    {
        for (const u32 id : ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = ecs->get_component<ScriptComponent>(id);
            if (!script->entity)
            {
                void* handle = ScriptingEngine::load_script(script->file_path);
                if (!handle)
                {
                    continue;
                }

                void* raw_create_script_fn = ScriptingEngine::get_symbol(handle, "create_script");
                void* raw_destroy_script_fn = ScriptingEngine::get_symbol(handle, "destroy_script");

                if (!raw_create_script_fn || !raw_destroy_script_fn)
                {
                    continue;
                }

                using CreateScriptFnPtr = ScriptableEntity* (*)();
                using DestroyScriptFnPtr = void (*)(ScriptableEntity*);

                CreateScriptFn create_script_fn = reinterpret_cast<CreateScriptFnPtr>(raw_create_script_fn);
                DestroyScriptFn destroy_script_fn = reinterpret_cast<DestroyScriptFnPtr>(raw_destroy_script_fn);

                script->handle = handle;
                script->create_entity = create_script_fn;
                script->destroy_entity = destroy_script_fn;

                script->entity = script->create_entity();
                script->entity->entity_id = id;
                script->entity->ecs = ecs.get();
                script->entity->physics_world = physics_world.get();
                script->entity->on_create();
            }
        }
    }

    void Scene::on_stop()
    {
        destroy_scripts();

        on_stop_internal();

        running = false;
    }

    void Scene::destroy_scripts()
    {
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity)
            {
                script->entity->on_destroy();
                script->destroy_entity(script->entity);
                script->entity = nullptr;

                ScriptingEngine::unload_script(script->handle);
                script->handle = nullptr;
            }
        }
    }

    void Scene::on_update(const f32 dt)
    {
        // Delete enqueued entities
        for (i32 i = entity_deletion_queue.size() - 1; i >= 0; i--)
        {
            const u32 entity_id = entity_deletion_queue[i];

            // Remove physics object from physics world if entity has physics properties
            auto [rigid_body, collider, transform] =
                ecs->get_components<RigidBodyComponent, BoxColliderComponent, TransformComponent>(entity_id);

            if (rigid_body && collider && transform)
            {
                physics_world->remove_rigid_body(rigid_body->collision_object);
            }

            ecs->erase_entity(entity_id);
        }

        entity_deletion_queue.clear();

        if (running)
        {
            // Update physics world
            physics_world->on_update(dt);

            // Synchronize physics components with the physics world
            auto objects = ecs->get_all_components_of_types<TransformComponent, RigidBodyComponent>();

            for (auto [transform, rigid_body] : objects)
            {
                // Object has default scale, so we don't copy it
                physics_world->get_collision_object_transform(rigid_body->collision_object, transform->translation,
                                                              transform->rotation);
            }

            // Update scripts
            for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
            {
                if (script->entity)
                {
                    script->entity->on_update(dt);
                }
            }
        }

        else
        {
            // Update physics world without advancing the simulation
            physics_world->on_update(0);
        }

        on_update_internal(dt);
    }

    void Scene::on_component_added(const u32 id, Component* component)
    {
        // Add rigidbody to physics world if component is a rigidbody or collider
        const b8 is_rigid_body_component = dynamic_cast<RigidBodyComponent*>(component) != nullptr;
        const b8 is_collider_component = dynamic_cast<BoxColliderComponent*>(component) != nullptr;
        if (is_rigid_body_component || is_collider_component)
        {
            auto* transform = ecs->get_component<TransformComponent>(id);
            auto* rigid_body = ecs->get_component<RigidBodyComponent>(id);
            auto* collider = ecs->get_component<BoxColliderComponent>(id);
            if (transform && rigid_body && collider)
            {
                rigid_body->collision_object =
                    physics_world->add_rigid_body(transform->translation, quat(math::radians(transform->rotation)),
                                                  collider->dimensions, rigid_body->mass);
            }
        }

        on_component_added_internal(id, component);
    }

    void Scene::on_event(const Event& e)
    {
        dispatch_event<WindowResizeEvent>(e, BIND_FN(Scene::on_resize));

        // Emit events to the native scripts
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity)
            {
                script->entity->on_event(e);
            }
        }

        on_event_internal(e);
    }

    void Scene::on_resize(const WindowResizeEvent& e)
    {
        const uvec2& size = {e.width, e.height};

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(size);
        }
    }

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
        if (!ecs->entity_exists(id))
        {
            return;
        }

        // Enqueue entity
        entity_deletion_queue.push_back(id);
    }

    void Scene::set_name(const str& name) { this->name = name; }

    b8 Scene::is_running() const { return running; }

    const str& Scene::get_name() const { return name; }

    const PhysicsWorld* Scene::get_physics_world() const { return physics_world.get(); }

    ECS& Scene::get_ecs() { return *ecs; }

    Camera& Scene::get_camera()
    {
        // @TODO: for now we assume the active camera is the first entity with a camera component
        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            return camera_c->camera;
        }

        ASSERT(false, "No runtime camera!");
        return std::get<0>(components[0])->camera;
    }

    void Scene::on_start_internal() {}
    void Scene::on_stop_internal() {}
    void Scene::on_event_internal(const Event& e) { (void)e; }
    void Scene::on_update_internal(const f32 dt) { (void)dt; }
    void Scene::on_component_added_internal(const u32 id, Component* component)
    {
        (void)id;
        (void)component;
    }

    // @TODO
    // void Scene::set_render_scale(const f32 new_render_scale)
    // {
    //     (void)
    //     // render_scale = std::clamp(new_render_scale, 0.01f, 1.0f);

    //     // auto e = WindowResizeEvent(draw_size.x, draw_size.y);
    //     // on_resize(e);
    //     // // build_render_graph(draw_size);
    // }
};  // namespace mag
