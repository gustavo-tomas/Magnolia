#include "systems.hpp"

#include <magnolia/core/keys.hpp>
#include <magnolia/core/logger.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/texture.hpp>

#include "components.hpp"
#include "scene.hpp"

namespace game
{
    static void handle_movement(const f32 dt, Scene& scene, mag::physics::IPhysicsWorld& physics,
                                PerspectiveCameraComponent& camera, RigidBodyComponent& rigid_body);

    static void handle_shooting(Scene& scene, mag::ECS& ecs, mag::physics::IPhysicsWorld& physics,
                                TransformComponent& transform, const f32 dt);

    static void fire_bullet(Scene& scene, mag::ECS& ecs, mag::physics::IPhysicsWorld& physics,
                            const TransformComponent& transform);

    static void on_key_press(Scene& scene, const mag::Key& key);

    static void on_mouse_click(const mag::Button& button);

    static mag::EntityID create_entity(mag::ECS& ecs, const str& name);

    // @TODO: we are being a bit lazy here to avoid updating the scene serializer. Because there is tipically only one
    // player we can get away with this method until the player behaviour is more well defined. Then we can properly
    // make it an ECS component.

    static PlayerComponent player = {};

    void execute_systems(Scene& scene, const f32 dt)
    {
        player_system(scene, dt);
        bullet_system(scene, dt);
        enemy_system(scene, dt);
    }

    void player_system(Scene& scene, const f32 dt)
    {
        auto& ecs = scene.get_ecs();

        auto entities =
            ecs.get_all_components_of_types<TransformComponent, PerspectiveCameraComponent, RigidBodyComponent>();

        mag::physics::IPhysicsWorld& physics = scene.get_physics_world();

        for (auto& [transform, camera, rigid_body] : entities)
        {
            handle_movement(dt, scene, physics, *camera, *rigid_body);
            handle_shooting(scene, ecs, physics, *transform, dt);
        }
    }

    void handle_shooting(Scene& scene, mag::ECS& ecs, mag::physics::IPhysicsWorld& physics,
                         TransformComponent& transform, const f32 dt)
    {
        static f32 timer = 0;
        timer += dt;

        if (timer >= 1.0f / player.fire_rate && mag::window::is_button_down(mag::Button::Left))
        {
            fire_bullet(scene, ecs, physics, transform);
            timer = 0.0f;
        }
    }

    void handle_movement(const f32 dt, Scene& scene, mag::physics::IPhysicsWorld& physics,
                         PerspectiveCameraComponent& camera, RigidBodyComponent& rigid_body)
    {
        (void)dt;

        // @TODO: quick hack to set dof

        static b8 init = false;
        if (!init)
        {
            const mag::DegreesOfFreedom dof = mag::DegreesOfFreedom::TranslationX |
                                              mag::DegreesOfFreedom::TranslationY | mag::DegreesOfFreedom::TranslationZ;

            physics.set_degrees_of_freedom(rigid_body.rigid_body_handle, dof);

            init = true;
        }

        const f32 physics_dt = physics.get_fixed_delta_time();

        // Handle mouse inputs first
        if (mag::window::is_key_pressed(mag::Key::Tab))
        {
            on_key_press(scene, mag::Key::Tab);
        }

        if (mag::window::is_button_pressed(mag::Button::Right))
        {
            on_mouse_click(mag::Button::Right);
        }

        // @TODO: this might return incorrect values if the user turns too fast (hit the edge of the window). A more
        // precise solution might be needed to correctly calculate the mouse delta.

        if (mag::window::is_mouse_captured())
        {
            const mag::math::ivec2 mouse_position = mag::window::get_mouse_position();
            const mag::math::ivec2 window_center = mag::window::get_window_center();
            const mag::math::vec2 mouse_delta = window_center - mouse_position;

            // Rotate
            player.pitch += mouse_delta.y * player.mouse_sensitivity * physics_dt;
            player.yaw += mouse_delta.x * player.mouse_sensitivity * physics_dt;

            mag::window::set_mouse_position(window_center.x, window_center.y);
        }

        const mag::RigidBodyHandle rigid_body_handle = rigid_body.rigid_body_handle;

        // Reset linear velocity for X and Z axes
        vec3 new_velocity = physics.get_linear_velocity(rigid_body_handle);
        new_velocity.x = 0.0f;
        new_velocity.z = 0.0f;

        physics.set_linear_velocity(rigid_body_handle, new_velocity);

        // Get current velocity
        const mag::vec3& velocity = physics.get_linear_velocity(rigid_body_handle);

        // Calculate desired movement direction
        const mag::vec3& forward = mag::math::get_forward_dir(player.pitch, player.yaw);
        const mag::vec3& right = mag::math::get_right_dir(player.yaw);

        mag::vec3 input_direction(0.0f);

        if (mag::window::is_key_down(mag::Key::w))
        {
            input_direction -= forward;
        }
        if (mag::window::is_key_down(mag::Key::s))
        {
            input_direction += forward;
        }
        if (mag::window::is_key_down(mag::Key::a))
        {
            input_direction -= right;
        }
        if (mag::window::is_key_down(mag::Key::d))
        {
            input_direction += right;
        }

        // Prevent nan values
        if (length(input_direction) > 0.0f)
        {
            input_direction = normalize(input_direction);

            // Set horizontal velocity directly
            mag::vec3 new_velocity = input_direction * player.walk_speed * physics_dt;
            new_velocity.y = velocity.y;  // Preserve vertical velocity

            physics.set_linear_velocity(rigid_body_handle, new_velocity);
        }

        // Update the camera transform
        mag::quat new_rot = mag::quat({player.pitch, player.yaw, 0.0f});
        new_rot = mag::math::normalize(new_rot);

        mag::vec3 new_pos(0.0f);
        physics.get_position(rigid_body_handle, new_pos);

        camera.camera.set_rotation(new_rot);
        camera.camera.set_position(new_pos + forward * player.camera_offset);
    }

    void fire_bullet(Scene& scene, mag::ECS& ecs, mag::physics::IPhysicsWorld& physics,
                     const TransformComponent& transform)
    {
        const mag::vec3& forward_dir = mag::math::get_forward_dir(player.pitch, player.yaw);

        // Create a bullet
        static u32 counter = 0;
        const u32 bullet_id = create_entity(ecs, "Bullet_" + std::to_string(counter++));

        // Apply small offset to avoid collisions with the player
        TransformComponent bullet_transform = transform;
        bullet_transform.scale = mag::vec3(0.01f);
        bullet_transform.translation -= forward_dir * player.bullet_offset;

        const str file_path = "test_game/assets/sprites/test_texture0.png";
        mag::resource::get_texture_async(file_path, scene.get_job_group(),
                                         [&ecs, file_path, bullet_id](const mag::ref<mag::IResource>& resource)
        {
            auto res = std::dynamic_pointer_cast<mag::TextureResource>(resource);

            ecs.add_component<SpriteComponent>(bullet_id, res);
        }, false);

        const f32 radius = 2.5f;
        const f32 height = 0.0f;
        const f32 mass = 10.0f;
        const f32 impulse = 1000.0f;
        const f32 time_to_live = 10.0f;

        const CapsuleCollider collider = CapsuleCollider(radius, height);
        ecs.add_component<TransformComponent>(bullet_id, bullet_transform);
        ecs.add_component<RigidBodyComponent>(bullet_id, collider, mass);
        ecs.add_component<BulletComponent>(bullet_id, time_to_live);

        auto* bullet_rigid_body = ecs.get_component<RigidBodyComponent>(bullet_id);

        physics.apply_impulse(bullet_rigid_body->rigid_body_handle, -forward_dir * impulse);
    }

    void on_key_press(Scene& scene, const mag::Key& key)
    {
        // Swap scenes
        if (key == mag::Key::Tab)
        {
            scene.set_next_scene("test_game/assets/scenes/Sponza.mag.json");
        }
    }

    void on_mouse_click(const mag::Button& button)
    {
        // Capture/Release the cursor
        if (button == mag::Button::Right)
        {
            const b8 capture = !mag::window::is_mouse_captured();

            mag::window::set_capture_mouse(capture);

            // Keep mouse button centered
            if (capture)
            {
                const mag::math::ivec2 window_center = mag::window::get_window_center();
                mag::window::set_mouse_position(window_center.x, window_center.y);
            }
        }
    }

    mag::EntityID create_entity(mag::ECS& ecs, const str& name)
    {
        const mag::EntityID entity_id = ecs.create_entity();

        if (!name.empty())
        {
            ecs.add_component<NameComponent>(entity_id, name);
        }

        return entity_id;
    }

    void bullet_system(Scene& scene, const f32 dt)
    {
        auto& ecs = scene.get_ecs();

        const std::vector<mag::EntityID>& entities_ids =
            ecs.query<TransformComponent, RigidBodyComponent, BulletComponent>();

        for (const u32 entity_id : entities_ids)
        {
            auto [transform, rigid_body, bullet] =
                ecs.get_components<TransformComponent, RigidBodyComponent, BulletComponent>(entity_id);

            bullet->time_to_live -= dt;

            if (bullet->time_to_live <= 0.0f)
            {
                scene.remove_entity(entity_id);
            }
        }
    }

    // @TODO: quick hack for testing

    void enemy_system(Scene& scene, const f32 dt)
    {
        (void)dt;

        auto& ecs = scene.get_ecs();
        auto& physics = scene.get_physics_world();

        static b8 init = false;
        static mag::EntityID enemy_id = mag::Invalid_ID;

        if (!ecs.entity_exists(enemy_id))
        {
            init = false;
        }

        if (scene.get_file_path() != "test_game/assets/scenes/Main.mag.json")
        {
            init = false;
            return;
        }

        if (!init)
        {
            enemy_id = ecs.create_entity();

            const str file_path = "test_game/assets/models/enemy/native/Enemy.model.json";
            const f32 mass = 1.0f;
            const vec3 scale = vec3(10.0f);

            Collider collider = BoxCollider(scale);

            ecs.add_component<TransformComponent>(enemy_id, vec3(-40, scale.y, 0), quat(1, 0, 0, 0), scale);
            ecs.add_component<RigidBodyComponent>(enemy_id, collider, mass);
            ecs.add_component<EnemyComponent>(enemy_id);

            mag::resource::get_model_async(file_path, scene.get_job_group(),
                                           [&ecs, file_path](const mag::ref<mag::IResource>& resource)
            {
                auto res = std::dynamic_pointer_cast<mag::ModelResource>(resource);

                ecs.add_component<ModelComponent>(enemy_id, res);
            }, false);

            init = true;
        }

        // Check for bullet hits

        const auto* rigid_body = ecs.get_component<RigidBodyComponent>(enemy_id);
        auto* enemy = ecs.get_component<EnemyComponent>(enemy_id);

        std::vector<mag::RigidBodyHandle> collisions;
        physics.get_rigid_body_collisions(rigid_body->rigid_body_handle, collisions);

        // @TODO: inefficient, for testing only
        auto bullets = ecs.get_all_components_of_types<RigidBodyComponent, BulletComponent>();
        for (const mag::RigidBodyHandle& collision_handle : collisions)
        {
            for (const auto& [bullet_body, bullet] : bullets)
            {
                if (bullet_body->rigid_body_handle == collision_handle && bullet->time_to_live > 0.0f)
                {
                    enemy->hp -= bullet->damage;
                    bullet->time_to_live = 0.0f;

                    LOG_INFO("HP: {0}", enemy->hp);

                    break;
                }
            }
        }

        if (enemy->hp <= 0.0f)
        {
            LOG_INFO("DEAD");
        }
    }
};  // namespace game
