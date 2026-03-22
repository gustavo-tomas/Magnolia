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
#include <magnolia/threads/job_system.hpp>

#include "ecs/components.hpp"
#include "ecs/debug.hpp"
#include "ecs/systems.hpp"
#include "renderer.hpp"
#include "scriptable_entity.hpp"

namespace game
{
    Scene::Scene(Renderer* renderer)
        : ecs(mag::create_unique<mag::ECS>([this](const mag::EntityID id, std::any component)
    { on_component_added(id, component); })),
          physics_world(mag::physics::create_physics_world()),
          renderer(renderer),
          job_group(mag::thread::create_job_group())
    {
    }

    Scene::~Scene()
    {
        // Destroy the job group
        mag::thread::destroy_job_group(job_group);

        if (running)
        {
            on_stop();
        }
    }

    void Scene::on_start()
    {
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
            physics_world->get_position_and_rotation(rigid_body->rigid_body_handle, transform->translation,
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

        // Update systems
        execute_systems(*this, dt);
    }

    void Scene::on_render(const f32 dt)
    {
#if MAG_CONFIG_DEBUG
        debug_system(*this, dt);
#endif
    }

    void Scene::on_component_added(const mag::EntityID id, std::any& component)
    {
        const b8 is_transform = component.type() == typeid(TransformComponent);
        const b8 is_rigid_body = component.type() == typeid(RigidBodyComponent);
        const b8 is_script = component.type() == typeid(ScriptComponent);
        const b8 is_model = component.type() == typeid(ModelComponent);
        const b8 is_sprite = component.type() == typeid(SpriteComponent);
        const b8 is_text = component.type() == typeid(TextComponent);

        // Add rigidbody to physics world
        if (is_rigid_body)
        {
            auto* rigid_body = ecs->get_component<RigidBodyComponent>(id);

            vec3 position = vec3(0.0f);
            quat rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
            vec3 scale = vec3(1.0f);

            if (auto* transform = ecs->get_component<TransformComponent>(id))
            {
                position = transform->translation;
                rotation = transform->rotation;
                scale = transform->scale;
            }

            if (auto* collider = std::get_if<BoxCollider>(&rigid_body->collider))
            {
                const vec3 dimensions = collider->dimensions;

                rigid_body->rigid_body_handle =
                    physics_world->add_rigid_body(position, rotation, dimensions, rigid_body->mass);
            }

            else if (auto* collider = std::get_if<CapsuleCollider>(&rigid_body->collider))
            {
                const f32 radius = collider->radius;
                const f32 height = collider->height;

                rigid_body->rigid_body_handle =
                    physics_world->add_rigid_body(position, rotation, radius, height, rigid_body->mass);
            }

            else if (auto* collider = std::get_if<MeshCollider>(&rigid_body->collider))
            {
                // Scale the dimensions of the collider to match the transform
                for (mag::math::Triangle& triangle : collider->triangles)
                {
                    triangle.v0 *= scale;
                    triangle.v1 *= scale;
                    triangle.v2 *= scale;
                }

                rigid_body->rigid_body_handle =
                    physics_world->add_rigid_body(position, rotation, rigid_body->mass, collider->triangles);
            }

            else
            {
                MAG_ASSERT(false, "Missing collider for rigidbody: '{0}'", static_cast<void*>(rigid_body));
            }

            return;
        }

        // Set rigidbody transforms
        if (is_transform)
        {
            auto* transform = ecs->get_component<TransformComponent>(id);

            if (auto* rigid_body = ecs->get_component<RigidBodyComponent>(id))
            {
                physics_world->set_position(rigid_body->rigid_body_handle, transform->translation);
                physics_world->set_rotation(rigid_body->rigid_body_handle, transform->rotation);
            }
        }

        // Instantiate scripts during runtime
        if (is_script && is_running())
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

    const mag::JobGroupHandle& Scene::get_job_group() const { return job_group; }

    mag::physics::IPhysicsWorld& Scene::get_physics_world() { return *physics_world; }

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
