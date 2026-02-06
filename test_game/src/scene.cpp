#include "scene.hpp"

#include <magnolia/audio/audio_system.hpp>
#include <magnolia/camera/camera.hpp>
#include <magnolia/core/assert.hpp>
#include <magnolia/core/event.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/math/types.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/scripting/scripting_engine.hpp>

#include "components.hpp"
#include "renderer.hpp"
#include "scriptable_entity.hpp"

namespace game
{
    Scene::Scene(Renderer* renderer)
        : ecs(mag::create_unique<mag::ECS>([this](const mag::EntityID id, std::any component)
                                           { on_component_added(id, component); })),
          physics_world(mag::physics::create_physics_world()),
          renderer(renderer)
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
#if MAG_CONFIG_DEBUG
        // Add debug scripts
        const mag::EntityID id = ecs->create_entity();
        ecs->add_component<DebugComponent>(id);
        ecs->add_component<ScriptComponent>(id, "test_game/assets/scripts/debug.cpp");
#endif

        // Instantiate scripts
        for (const mag::EntityID id : ecs->query<ScriptComponent>())
        {
            create_script(id);
        }

        // Play audios
        for (const AudioComponent* audio_c : ecs->get_all_components_of_type<AudioComponent>())
        {
            if (audio_c->play_on_load)
            {
                mag::ref<mag::AudioResource> audio = mag::resource::get_audio(audio_c->audio->file_path);
                mag::audio::play(audio, audio_c->volume, audio_c->position, audio_c->velocity);
            }
        }

        running = true;
    }

    void Scene::create_script(const mag::EntityID id)
    {
        ScriptComponent* script = ecs->get_component<ScriptComponent>(id);

        // Already instantiated
        if (script->entity != nullptr)
        {
            return;
        }

        mag::script::RecompileScriptParams script_params = {};
        script_params.file_path = script->file_path;

        // Recompile if necessary
        if (!mag::script::compile_script(script_params))
        {
            return;
        }

        // Now we can safely load
        const mag::script::ScriptHandle handle = mag::script::load_script(script_params.file_path);
        if (handle == mag::Invalid_ID)
        {
            return;
        }

        void* raw_create_script_fn = mag::script::get_symbol(handle, "create_script");
        void* raw_destroy_script_fn = mag::script::get_symbol(handle, "destroy_script");

        if ((raw_create_script_fn == nullptr) || (raw_destroy_script_fn == nullptr))
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
        if (script->entity == nullptr)
        {
            return;
        }

        script->entity->on_destroy();
        script->destroy_entity(script->entity);
        script->entity = nullptr;
    }

    void Scene::on_stop()
    {
        // @TODO @NOTE: virtual functions may become inaccessible after a dlclose. This causes the ECS to crash during
        // deletion. To prevent this, we have to delay the dll unloading until the ECS is destroyed. This has to be done
        // for every component added and/or data allocated. A better solution must be found to fix this issue.

        // See: https://stackoverflow.com/questions/36524043/accessing-memory-allocated-by-shared-library-after-dlclose

        // Stop audios
        for (auto* audio : ecs->get_all_components_of_type<AudioComponent>())
        {
            mag::audio::stop(audio->audio);
        }

        std::vector<ScriptComponent> scripts;

        // Copy script data before ECS deletion
        for (auto* script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            scripts.push_back(*script);
        }

        // Destroy the ECS
        ecs.reset();

        // Destroy instantiated scripts and unload dlls
        for (auto& script : scripts)
        {
            destroy_script(&script);
            mag::script::unload_script(script.handle);
        }

        running = false;
    }

    void Scene::on_update(const f32 dt)
    {
        // Delete enqueued entities
        for (i32 i = static_cast<i32>(entity_deletion_queue.size()) - 1; i >= 0; i--)
        {
            const mag::EntityID entity_id = entity_deletion_queue[i];

            // Remove physics object from physics world if entity has physics properties
            RigidBodyComponent* rigid_body = ecs->get_component<RigidBodyComponent>(entity_id);

            if (rigid_body != nullptr)
            {
                physics_world->remove_rigid_body(rigid_body->rigid_body_handle);
            }

            // Delete script instance if entity has a script component
            ScriptComponent* script = ecs->get_component<ScriptComponent>(entity_id);

            if (script != nullptr)
            {
                destroy_script(script);
            }

            ecs->erase_entity(entity_id);
        }

        entity_deletion_queue.clear();

        // Update physics world
        physics_world->on_update(dt);

        // Synchronize physics components with the physics world
        auto objects = ecs->get_all_components_of_types<TransformComponent, RigidBodyComponent>();

        for (auto [transform, rigid_body] : objects)
        {
            // Object has default scale, so we don't copy it
            physics_world->get_collision_object_transform(rigid_body->rigid_body_handle, transform->translation,
                                                          transform->rotation);
        }

        // Update scripts
        for (auto* script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity != nullptr)
            {
                script->entity->on_update(dt);
            }
        }
    }

    void Scene::on_render(const f32 dt)
    {
        for (auto* script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity != nullptr)
            {
                script->entity->on_render(dt);
            }
        }
    }

    void Scene::on_component_added(const mag::EntityID id, std::any& component)
    {
        const b8 is_rigid_body = component.type() == typeid(RigidBodyComponent);
        const b8 is_script = component.type() == typeid(ScriptComponent);
        const b8 is_model = component.type() == typeid(ModelComponent);
        const b8 is_sprite = component.type() == typeid(SpriteComponent);
        const b8 is_text = component.type() == typeid(TextComponent);

        // @TODO: this code is quite brittle and assumes that a rigidbody component must be preceded by a transform and
        // a collider, else it doesn't work.

        // Add rigidbody to physics world
        if (is_rigid_body)
        {
            auto* transform = ecs->get_component<TransformComponent>(id);
            auto* rigid_body = ecs->get_component<RigidBodyComponent>(id);
            auto* box_collider = ecs->get_component<BoxColliderComponent>(id);
            auto* capsule_collider = ecs->get_component<CapsuleColliderComponent>(id);
            auto* mesh_collider = ecs->get_component<MeshColliderComponent>(id);

            const vec3 position = transform->translation;
            const quat rotation = transform->rotation;
            const vec3 scale = transform->scale;

            if (box_collider != nullptr)
            {
                const vec3 dimensions = box_collider->dimensions;

                rigid_body->rigid_body_handle =
                    physics_world->add_rigid_body(position, rotation, dimensions, rigid_body->mass);
            }

            else if (capsule_collider != nullptr)
            {
                const f32 radius = capsule_collider->radius;
                const f32 height = capsule_collider->height;

                rigid_body->rigid_body_handle =
                    physics_world->add_rigid_body(position, rotation, radius, height, rigid_body->mass);
            }

            else if (mesh_collider != nullptr)
            {
                // Scale the dimensions of the collider to match the transform
                for (mag::math::Triangle& triangle : mesh_collider->triangles)
                {
                    triangle.v0 *= scale;
                    triangle.v1 *= scale;
                    triangle.v2 *= scale;
                }

                rigid_body->rigid_body_handle =
                    physics_world->add_rigid_body(position, rotation, rigid_body->mass, mesh_collider->triangles);
            }

            else
            {
                MAG_ASSERT(false, "Missing collider for rigidbody: '{0}'", static_cast<void*>(rigid_body));
            }

            return;
        }

        // Instantiate scripts during runtime
        if (is_running() && is_script)
        {
            create_script(id);
            return;
        }

        // Upload model data to the GPU
        if (is_model)
        {
            auto* model = ecs->get_component<ModelComponent>(id);
            renderer->on_model_added(*model->model);
            return;
        }

        // Upload texture data to the GPU
        if (is_sprite)
        {
            auto* sprite = ecs->get_component<SpriteComponent>(id);
            renderer->on_texture_added(*sprite->texture);
            return;
        }

        // Upload font data to the GPU
        if (is_text)
        {
            auto* text = ecs->get_component<TextComponent>(id);
            renderer->on_font_added(*text->font);
            return;
        }
    }

    void Scene::on_event(const mag::Event& e)
    {
        dispatch_event<mag::WindowResizeEvent>(e, [this](const mag::WindowResizeEvent& e) { on_resize(e); });

        // Emit events to the native scripts
        for (auto* script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity != nullptr)
            {
                script->entity->on_event(e);
            }
        }
    }

    void Scene::on_resize(const mag::WindowResizeEvent& e)
    {
        const uvec2 size = {e.width, e.height};

        for (auto* camera_c : ecs->get_all_components_of_type<PerspectiveCameraComponent>())
        {
            camera_c->camera.set_viewport_size(size);
        }

        for (auto* camera_c : ecs->get_all_components_of_type<OrthographicCameraComponent>())
        {
            camera_c->camera.set_viewport_size(size);
        }
    }

    void Scene::remove_entity(const mag::EntityID id)
    {
        if (!ecs->entity_exists(id))
        {
            return;
        }

        // Enqueue entity
        entity_deletion_queue.push_back(id);
    }

    void Scene::set_file_path(const str& file_path) { this->file_path = file_path; }

    void Scene::set_name(const str& name) { this->name = name; }

    b8 Scene::is_running() const { return running; }

    const str& Scene::get_name() const { return name; }

    const str& Scene::get_file_path() const { return file_path; }

    const str& Scene::get_next_scene() const { return next_scene; }

    const mag::physics::IPhysicsWorld* Scene::get_physics_world() const { return physics_world.get(); }

    mag::ECS& Scene::get_ecs() { return *ecs; }

    mag::Camera& Scene::get_camera()
    {
        // @TODO: for now we assume the active camera is the first entity with a camera component
        auto perspective_cameras = ecs->get_all_components_of_types<PerspectiveCameraComponent>();
        for (auto [camera_c] : perspective_cameras)
        {
            return camera_c->camera;
        }

        auto ortho_cameras = ecs->get_all_components_of_types<OrthographicCameraComponent>();
        for (auto [camera_c] : ortho_cameras)
        {
            return camera_c->camera;
        }

        MAG_ASSERT(false, "No runtime camera!");

        static mag::PerspectiveCamera default_camera({});
        return default_camera;
    }
};  // namespace game
