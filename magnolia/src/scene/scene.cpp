#include "magnolia/scene/scene.hpp"

#include "magnolia/audio/audio_system.hpp"
#include "magnolia/camera/camera.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/event.hpp"
#include "magnolia/ecs/components.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/physics/physics.hpp"
#include "magnolia/resources/audio.hpp"
#include "magnolia/scene/scriptable_entity.hpp"
#include "magnolia/scripting/scripting_engine.hpp"

namespace mag
{
    Scene::Scene()
        : name("Untitled"), ecs(new ECS(BIND_FN2(Scene::on_component_added))), physics_world(create_physics_world())
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
        // Instantiate scripts
        for (const u32 id : ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            create_script(id);
        }

        // Play audios
        for (const AudioComponent* audio_c : ecs->get_all_components_of_type<AudioComponent>())
        {
            if (audio_c->play_on_load)
            {
                ref<AudioResource> audio = resource::get_audio(audio_c->audio->file_path);
                audio::play(audio, audio_c->volume, audio_c->position, audio_c->velocity);
            }
        }

        running = true;
    }

    void Scene::create_script(const u32 id)
    {
        ScriptComponent* script = ecs->get_component<ScriptComponent>(id);

        // Already instantiated
        if (script->entity)
        {
            return;
        }

        // Recompile if necessary
        if (!script::recompile_script(script->file_path))
        {
            return;
        }

        // Now we can safely load
        void* handle = script::load_script(script->file_path);
        if (!handle)
        {
            return;
        }

        void* raw_create_script_fn = script::get_symbol(handle, "create_script");
        void* raw_destroy_script_fn = script::get_symbol(handle, "destroy_script");

        if (!raw_create_script_fn || !raw_destroy_script_fn)
        {
            return;
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
        script->entity->scene = this;
        script->entity->on_create();
    }

    void Scene::destroy_script(ScriptComponent* script)
    {
        if (!script->entity)
        {
            return;
        }

        script->entity->on_destroy();
        script->destroy_entity(script->entity);
        script->entity = nullptr;

        script::unload_script(script->handle);
        script->handle = nullptr;
    }

    void Scene::on_stop()
    {
        // Destroy instantiated scripts
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            destroy_script(script);
        }

        // Stop audios
        for (auto audio : ecs->get_all_components_of_type<AudioComponent>())
        {
            mag::audio::stop(audio->audio);
        }

        running = false;
    }

    void Scene::on_update(const f32 dt)
    {
        // Delete enqueued entities
        for (i32 i = entity_deletion_queue.size() - 1; i >= 0; i--)
        {
            const u32 entity_id = entity_deletion_queue[i];

            // Remove physics object from physics world if entity has physics properties
            auto [rigid_body, collider, transform] =
                ecs->get_components<RigidBodyComponent, ColliderComponent, TransformComponent>(entity_id);

            if (rigid_body && collider && transform)
            {
                physics_world->remove_rigid_body(rigid_body->collision_object);
            }

            // Delete script instance if entity has a script component
            ScriptComponent* script = ecs->get_component<ScriptComponent>(entity_id);

            if (script)
            {
                destroy_script(script);
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
    }

    void Scene::on_component_added(const u32 id, Component* component)
    {
        // Add rigidbody to physics world if component is a rigidbody or collider
        const b8 is_rigid_body_component = dynamic_cast<RigidBodyComponent*>(component) != nullptr;
        const b8 is_collider_component = dynamic_cast<ColliderComponent*>(component) != nullptr;
        if (is_rigid_body_component || is_collider_component)
        {
            auto* transform = ecs->get_component<TransformComponent>(id);
            auto* rigid_body = ecs->get_component<RigidBodyComponent>(id);
            auto* collider = ecs->get_component<ColliderComponent>(id);
            if (transform && rigid_body && collider)
            {
                switch (collider->collider_type)
                {
                    case ColliderComponent::ColliderType::Box:
                    {
                        const vec3 dimensions = collider->collider.box.dimensions;
                        rigid_body->collision_object = physics_world->add_rigid_body(
                            transform->translation, quat(transform->rotation), dimensions, rigid_body->mass);
                    }
                    break;

                    case ColliderComponent::ColliderType::Capsule:
                    {
                        const f32 radius = collider->collider.capsule.radius;
                        const f32 height = collider->collider.capsule.height;
                        rigid_body->collision_object = physics_world->add_rigid_body(
                            transform->translation, quat(transform->rotation), radius, height, rigid_body->mass);
                    }
                    break;

                    default:
                        MAG_ASSERT(false, "Unhandled collider type");
                        break;
                }
            }
        }

        // Instantiate scripts during runtime
        const b8 is_script_component = dynamic_cast<ScriptComponent*>(component) != nullptr;
        if (is_running() && is_script_component)
        {
            create_script(id);
        }
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
    }

    void Scene::on_resize(const WindowResizeEvent& e)
    {
        const uvec2& size = {e.width, e.height};

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_viewport_size(size);
        }
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

    void Scene::set_next_scene(const str& scene_file_path) { next_scene = scene_file_path; }

    b8 Scene::is_running() const { return running; }

    const str& Scene::get_name() const { return name; }

    const str& Scene::get_next_scene() const { return next_scene; }

    const IPhysicsWorld* Scene::get_physics_world() const { return physics_world.get(); }

    ECS& Scene::get_ecs() { return *ecs; }

    Camera& Scene::get_camera()
    {
        // @TODO: for now we assume the active camera is the first entity with a camera component
        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            return camera_c->camera;
        }

        MAG_ASSERT(false, "No runtime camera!");
        return std::get<0>(components[0])->camera;
    }
};  // namespace mag
