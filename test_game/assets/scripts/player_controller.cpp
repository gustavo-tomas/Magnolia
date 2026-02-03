#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/texture.hpp>

#include "common.hpp"

using namespace game;

class PlayerController : public ScriptableEntity
{
    private:
        f32 hp = 100.0f;
        f32 walk_speed = 1650.0f;
        f32 mouse_sensitivity = 0.002f;
        f32 fire_rate = 20.0f;  // per second
        mag::vec3 camera_offset = mag::vec3(50.0f);
        mag::vec3 bullet_offset = mag::vec3(50.0f);

        // We can use the camera params instead of these
        f32 pitch = 0.0f;
        f32 yaw = 0.0f;

    public:
        void on_create() override
        {
            const RigidBodyComponent* rigid_body_c = get_component<RigidBodyComponent>();

            if (rigid_body_c == nullptr)
            {
                LOG_WARNING("Missing rigidbody");
                return;
            }

            mag::physics::IPhysicsWorld& physics = get_physics_world();

            // Prevent player from sleeping
            physics.set_activation_state(rigid_body_c->rigid_body_handle, mag::ActivationState::Activate);

            LOG_SUCCESS("Created PlayerController");
        }

        void on_destroy() override { LOG_SUCCESS("Destroyed PlayerController"); }

        void on_update(const f32 dt) override
        {
            handle_movement(dt);
            handle_shooting(dt);
        }

        void on_signal_received(const u32 sender_id, const void* data) override
        {
            // Damaged by some enemy
            const DamageData* damage_data = static_cast<const DamageData*>(data);
            if (damage_data != nullptr)
            {
                hp -= damage_data->damage;

                LOG_INFO("Damage: {0:.2} sent from: {1}", damage_data->damage, sender_id);
                LOG_INFO("Player HP: {0:.2}", hp);
            }
        }

        void handle_shooting(const f32 dt)
        {
            auto [transform] = get_components<TransformComponent>();
            if (transform == nullptr)
            {
                return;
            }

            static f32 timer = 0;
            timer += dt;
            if (timer >= 1.0f / fire_rate && mag::window::is_button_down(mag::Button::Left))
            {
                fire_bullet(*transform);
                timer = 0.0f;
            }
        }

        void handle_movement(const f32 dt)
        {
            auto [transform, camera_c, rigid_body_c] =
                get_components<TransformComponent, PerspectiveCameraComponent, RigidBodyComponent>();

            if (transform == nullptr || camera_c == nullptr || rigid_body_c == nullptr)
            {
                LOG_WARNING("Missing components");
                return;
            }

            // @TODO: this might return incorrect values if the user turns too fast (hit the edge of the window). A more
            // precise solution might be needed to correctly calculate the mouse delta.

            // Handle mouse inputs first
            if (mag::window::is_mouse_captured())
            {
                const mag::math::ivec2 mouse_position = mag::window::get_mouse_position();
                const mag::math::ivec2 window_center = mag::window::get_window_center();
                const mag::math::vec2 mouse_delta = window_center - mouse_position;

                // Rotate
                pitch += mouse_delta.y * mouse_sensitivity;
                yaw += mouse_delta.x * mouse_sensitivity;

                mag::window::set_mouse_position(window_center.x, window_center.y);
            }

            mag::physics::IPhysicsWorld& physics = get_physics_world();

            const mag::RigidBodyHandle rigid_body_handle = rigid_body_c->rigid_body_handle;

            physics.set_linear_velocity(rigid_body_handle, mag::vec3(0.0f));

            // Get current velocity
            const mag::vec3& velocity = physics.get_linear_velocity(rigid_body_handle);

            // Calculate desired movement direction
            const mag::vec3& forward = get_forward_dir();
            const mag::vec3& right = get_right_dir();

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
                mag::vec3 new_velocity = input_direction * walk_speed * dt;
                new_velocity.y = (velocity.y);  // Preserve vertical velocity

                physics.set_linear_velocity(rigid_body_handle, new_velocity);
            }

            // Update the camera transform
            mag::quat new_rot = mag::quat();
            mag::vec3 new_pos(0.0f);
            physics.get_collision_object_transform(rigid_body_handle, new_pos, new_rot);
            new_rot = mag::vec3(pitch, yaw, 0.0f);

            camera_c->camera.set_rotation(mag::math::eulerAngles(new_rot));
            camera_c->camera.set_position(new_pos + forward * camera_offset);
        }

        void fire_bullet(const TransformComponent& transform)
        {
            mag::physics::IPhysicsWorld& physics = get_physics_world();

            const mag::vec3& forward_dir = get_forward_dir();

            // Create a bullet
            static u32 counter = 0;
            const u32 bullet_id = create_entity("Bullet_" + std::to_string(counter++));

            // Apply small offset to avoid collisions with the player
            TransformComponent bullet_transform = transform;
            bullet_transform.scale = mag::vec3(0.01f);
            bullet_transform.translation -= forward_dir * bullet_offset;

            const str file_path = "test_game/assets/sprites/test_texture0.png";
            mag::resource::get_texture_async(
                file_path,
                [this, file_path, bullet_id](const mag::ref<mag::IResource>& resource)
                {
                    auto res = std::dynamic_pointer_cast<mag::TextureResource>(resource);

                    add_component_to_entity<SpriteComponent>(bullet_id, res);
                },
                false);

            ColliderComponent::Collider collider = {};
            collider.capsule.radius = 2.5f;
            collider.capsule.height = 0.0f;

            const f32 mass = 10.0f;
            const f32 impulse = 1000.0f;

            add_component_to_entity<TransformComponent>(bullet_id, bullet_transform);
            add_component_to_entity<RigidBodyComponent>(bullet_id, mass);
            add_component_to_entity<ColliderComponent>(bullet_id, ColliderComponent::ColliderType::Capsule, collider);

            auto [bullet_rigid_body] = get_external_entity_components<RigidBodyComponent>(bullet_id);

            physics.apply_impulse(bullet_rigid_body->rigid_body_handle, -forward_dir * impulse);
        }

        void on_event(const mag::Event& e) override
        {
            dispatch_event<mag::MousePressEvent>(e, [this](const mag::MousePressEvent& e) { on_mouse_click(e); });
            dispatch_event<mag::KeyPressEvent>(e, [this](const mag::KeyPressEvent& e) { on_key_press(e); });
        }

        void on_key_press(const mag::KeyPressEvent& e)
        {
            // Swap scenes
            if (e.key == mag::Key::Tab)
            {
                set_active_scene("test_game/assets/scenes/Sponza.mag.json");
            }
        }

        void on_mouse_click(const mag::MousePressEvent& e)
        {
            // Capture/Release the cursor
            if (e.button == mag::Button::Right)
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

        mag::vec3 get_right_dir() const
        {
            mag::vec3 right(0.0f);
            right.x = cos(yaw);
            right.y = 0;
            right.z = -sin(yaw);

            return right;
        }

        mag::vec3 get_forward_dir() const
        {
            mag::vec3 forward(0.0f);
            forward.x = cos(-pitch) * sin(yaw);
            forward.y = sin(-pitch);
            forward.z = cos(-pitch) * cos(yaw);

            return forward;
        }
};

extern "C" ScriptableEntity* create_script() { return new PlayerController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
