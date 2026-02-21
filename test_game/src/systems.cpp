#include "systems.hpp"

#include <magnolia/core/keys.hpp>
#include <magnolia/core/logger.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/window.hpp>

#include "components.hpp"
#include "scene.hpp"

namespace game
{
    static void handle_movement(const f32 dt, Scene& scene, mag::physics::IPhysicsWorld& physics,
                                PerspectiveCameraComponent& camera, RigidBodyComponent& rigid_body);

    static void handle_shooting(mag::ECS& ecs, mag::physics::IPhysicsWorld& physics, TransformComponent& transform,
                                const f32 dt);

    static void fire_bullet(mag::ECS& ecs, mag::physics::IPhysicsWorld& physics, const TransformComponent& transform);

    static void on_key_press(Scene& scene, const mag::Key& key);

    static void on_mouse_click(const mag::Button& button);

    static vec3 get_right_dir(const f32 yaw);
    static vec3 get_forward_dir(const f32 pitch, const f32 yaw);
    static vec3 get_up_dir(const f32 pitch, const f32 yaw);

    static mag::EntityID create_entity(mag::ECS& ecs, const str& name);

    // @TODO: we are being a bit lazy here to avoid updating the scene serializer. Because there is tipically only one
    // player we can get away with this method until the player behaviour is more well defined. Then we can properly
    // make it an ECS component.

    static PlayerComponent player = {};

    void player_system(Scene& scene, const f32 dt)
    {
        auto& ecs = scene.get_ecs();

        auto entities =
            ecs.get_all_components_of_types<TransformComponent, PerspectiveCameraComponent, RigidBodyComponent>();

        mag::physics::IPhysicsWorld& physics = scene.get_physics_world();

        for (auto& [transform, camera, rigid_body] : entities)
        {
            handle_movement(dt, scene, physics, *camera, *rigid_body);
            handle_shooting(ecs, physics, *transform, dt);

            if (mag::window::is_key_pressed(mag::Key::f))
            {
                mag::window::set_target_frame_rate(dt < 1.0f / 120.0f ? 120 : -1);
            }
        }
    }

    void handle_shooting(mag::ECS& ecs, mag::physics::IPhysicsWorld& physics, TransformComponent& transform,
                         const f32 dt)
    {
        static f32 timer = 0;
        timer += dt;

        if (timer >= 1.0f / player.fire_rate && mag::window::is_button_down(mag::Button::Left))
        {
            fire_bullet(ecs, physics, transform);
            timer = 0.0f;
        }
    }

    void handle_movement(const f32 dt, Scene& scene, mag::physics::IPhysicsWorld& physics,
                         PerspectiveCameraComponent& camera, RigidBodyComponent& rigid_body)
    {
        // @TODO: this might return incorrect values if the user turns too fast (hit the edge of the window). A more
        // precise solution might be needed to correctly calculate the mouse delta.

        // Handle mouse inputs first
        if (mag::window::is_key_pressed(mag::Key::Tab))
        {
            on_key_press(scene, mag::Key::Tab);
        }

        if (mag::window::is_button_pressed(mag::Button::Right))
        {
            on_mouse_click(mag::Button::Right);
        }

        if (mag::window::is_mouse_captured())
        {
            const mag::math::ivec2 mouse_position = mag::window::get_mouse_position();
            const mag::math::ivec2 window_center = mag::window::get_window_center();
            const mag::math::vec2 mouse_delta = window_center - mouse_position;

            // Rotate
            player.pitch += mouse_delta.y * player.mouse_sensitivity * dt;
            player.yaw += mouse_delta.x * player.mouse_sensitivity * dt;

            mag::window::set_mouse_position(window_center.x, window_center.y);
        }

        const mag::RigidBodyHandle rigid_body_handle = rigid_body.rigid_body_handle;

        // Reset velocity for X and Z axes
        vec3 new_velocity = physics.get_linear_velocity(rigid_body_handle);
        new_velocity.x = 0.0f;
        new_velocity.z = 0.0f;

        physics.set_linear_velocity(rigid_body_handle, new_velocity);

        // Get current velocity
        const mag::vec3& velocity = physics.get_linear_velocity(rigid_body_handle);

        // Calculate desired movement direction
        const mag::vec3& forward = get_forward_dir(player.pitch, player.yaw);
        const mag::vec3& right = get_right_dir(player.yaw);

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
            mag::vec3 new_velocity = input_direction * player.walk_speed * dt;
            new_velocity.y = velocity.y;  // Preserve vertical velocity

            physics.set_linear_velocity(rigid_body_handle, new_velocity);
        }

        // Update the camera transform
        mag::quat new_rot = mag::quat();
        mag::vec3 new_pos(0.0f);
        physics.get_collision_object_transform(rigid_body_handle, new_pos, new_rot);
        new_rot = vec3(player.pitch, player.yaw, 0.0f);
        new_rot = mag::math::normalize(new_rot);

        camera.camera.set_rotation(new_rot);
        camera.camera.set_position(new_pos + forward * player.camera_offset);
    }

    void fire_bullet(mag::ECS& ecs, mag::physics::IPhysicsWorld& physics, const TransformComponent& transform)
    {
        const mag::vec3& forward_dir = get_forward_dir(player.pitch, player.yaw);

        // Create a bullet
        static u32 counter = 0;
        const u32 bullet_id = create_entity(ecs, "Bullet_" + std::to_string(counter++));

        // Apply small offset to avoid collisions with the player
        TransformComponent bullet_transform = transform;
        bullet_transform.scale = mag::vec3(0.01f);
        bullet_transform.translation -= forward_dir * player.bullet_offset;

        // @TODO: figure out a better way to handle data flow
        // const str file_path = "test_game/assets/sprites/test_texture0.png";
        // mag::resource::get_texture_async(
        //     file_path, get_job_group(),
        //     [this, file_path, bullet_id](const mag::ref<mag::IResource>& resource)
        //     {
        //         auto res = std::dynamic_pointer_cast<mag::TextureResource>(resource);

        //         add_component_to_entity<SpriteComponent>(bullet_id, res);
        //     },
        //     false);

        const f32 radius = 2.5f;
        const f32 height = 0.0f;
        const f32 mass = 10.0f;
        const f32 impulse = 1000.0f;

        const CapsuleCollider collider = CapsuleCollider(radius, height);
        ecs.add_component<TransformComponent>(bullet_id, bullet_transform);
        ecs.add_component<RigidBodyComponent>(bullet_id, collider, mass);

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

    // @TODO: move to math
    mag::vec3 get_right_dir(const f32 yaw)
    {
        mag::vec3 right(0.0f);
        right.x = cos(yaw);
        right.y = 0;
        right.z = -sin(yaw);

        return right;
    }

    mag::vec3 get_forward_dir(const f32 pitch, const f32 yaw)
    {
        mag::vec3 forward(0.0f);
        forward.x = cos(-pitch) * sin(yaw);
        forward.y = sin(-pitch);
        forward.z = cos(-pitch) * cos(yaw);

        return forward;
    }

    mag::vec3 get_up_dir(const f32 pitch, const f32 yaw)
    {
        const mag::vec3 forward = get_forward_dir(pitch, yaw);
        const mag::vec3 right = get_right_dir(yaw);

        return mag::math::cross(forward, right);
    }
};  // namespace game
