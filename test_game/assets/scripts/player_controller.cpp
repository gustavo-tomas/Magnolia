#include <magnolia/physics/physics.hpp>
#include <magnolia/resources/model.hpp>

#include "common.hpp"

using namespace mag;

class PlayerController : public ScriptableEntity
{
    private:
        f32 hp = 100.0f;
        f32 walk_speed = 1650.0f;
        f32 mouse_sensitivity = 0.002f;
        vec3 camera_offset = vec3(50.0f);
        vec3 bullet_offset = vec3(50.0f);

        // We can use the camera params instead of these
        f32 pitch = 0.0f;
        f32 yaw = 0.0f;

    public:
        virtual void on_create() override
        {
            const RigidBodyComponent* rigid_body_c = get_component<RigidBodyComponent>();

            if (!rigid_body_c)
            {
                LOG_WARNING("Missing rigidbody");
                return;
            }

            IPhysicsWorld& physics = get_physics_world();

            // Prevent player from sleeping
            physics.set_activation_state(rigid_body_c->collision_object, ActivationState::DisableDeactivation);

            LOG_SUCCESS("Created PlayerController");
        }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed PlayerController"); }

        virtual void on_update(const f32 dt) override
        {
            handle_movement(dt);
            handle_shooting();
        }

        virtual void on_signal_received(const u32 sender_id, const void* data) override
        {
            // Damaged by some enemy
            const DamageData* damage_data = static_cast<const DamageData*>(data);
            if (damage_data)
            {
                hp -= damage_data->damage;

                printf("Damage: %.2f sent from: %u\n", damage_data->damage, sender_id);
                printf("Player HP: %.2f\n", hp);
            }
        }

        void handle_shooting()
        {
            auto [transform] = get_components<TransformComponent>();
            if (!transform)
            {
                return;
            }

            if (window::is_button_pressed(Button::Left))
            {
                fire_bullet(*transform);
            }
        }

        void handle_movement(const f32 dt)
        {
            auto [transform, camera_c, rigid_body_c] =
                get_components<TransformComponent, CameraComponent, RigidBodyComponent>();

            if (!transform || !camera_c || !rigid_body_c)
            {
                LOG_WARNING("Missing components");
                return;
            }

            IPhysicsWorld& physics = get_physics_world();

            void* collision_object = rigid_body_c->collision_object;

            physics.set_linear_velocity(collision_object, vec3(0.0f));

            // Get current velocity
            const vec3& velocity = physics.get_linear_velocity(collision_object);

            // Calculate desired movement direction
            const vec3& forward = get_forward_dir();
            const vec3& right = get_right_dir();

            vec3 input_direction(0.0f);

            if (window::is_key_down(Key::w)) input_direction -= forward;
            if (window::is_key_down(Key::s)) input_direction += forward;
            if (window::is_key_down(Key::a)) input_direction -= right;
            if (window::is_key_down(Key::d)) input_direction += right;

            // Prevent nan values
            if (length(input_direction) > 0.0f)
            {
                input_direction = normalize(input_direction);

                // Set horizontal velocity directly
                vec3 new_velocity = input_direction * walk_speed * dt;
                new_velocity.y = (velocity.y);  // Preserve vertical velocity

                physics.set_linear_velocity(collision_object, new_velocity);
            }

            // Update the camera transform
            vec3 new_rot = vec3(0.0f);
            vec3 new_pos = vec3(0.0f);
            physics.get_collision_object_transform(collision_object, new_pos, new_rot);
            new_rot = vec3(pitch, yaw, 0.0f);

            camera_c->camera.set_rotation(new_rot);
            camera_c->camera.set_position(new_pos + forward * camera_offset);
        }

        void fire_bullet(const TransformComponent& transform)
        {
            IPhysicsWorld& physics = get_physics_world();

            const vec3& forward_dir = get_forward_dir();

            // Create a bullet
            const u32 bullet_id = create_entity();

            // Apply small offset to avoid collisions with the player
            TransformComponent bullet_transform = TransformComponent(transform);
            bullet_transform.scale = vec3(100.0f);
            bullet_transform.translation -= forward_dir * bullet_offset;

            const ref<ModelResource> model =
                resource::get_model("test_game/assets/models/hammer/native/wooden_hammer_01.model.json");

            ColliderComponent::Collider collider = {};
            collider.capsule.radius = 5.0f;
            collider.capsule.height = 10.0f;

            const f32 mass = 10.0f;

            add_component_to_entity<TransformComponent>(bullet_id, bullet_transform);
            add_component_to_entity<ModelComponent>(bullet_id, model);
            add_component_to_entity<RigidBodyComponent>(bullet_id, mass);
            add_component_to_entity<ColliderComponent>(bullet_id, ColliderComponent::ColliderType::Capsule, collider);

            auto [bullet_rigid_body] = get_external_entity_components<RigidBodyComponent>(bullet_id);

            physics.apply_impulse(bullet_rigid_body->collision_object, -forward_dir * 1000.0f);
        }

        virtual void on_event(const Event& e) override
        {
            dispatch_event<MouseMoveEvent>(e, BIND_FN(PlayerController::on_mouse_move));
            dispatch_event<MousePressEvent>(e, BIND_FN(PlayerController::on_mouse_click));
            dispatch_event<KeyPressEvent>(e, BIND_FN(PlayerController::on_key_press));
        }

        void on_key_press(const KeyPressEvent& e)
        {
            // Swap scenes
            if (e.key == Key::Tab)
            {
                set_active_scene("test_game/assets/scenes/Sponza.mag.json");
            }
        }

        void on_mouse_click(const MousePressEvent& e)
        {
            // Capture/Release the cursor
            if (e.button == Button::Right)
            {
                window::set_capture_mouse(!window::is_mouse_captured());
            }
        }

        void on_mouse_move(const MouseMoveEvent& e)
        {
            // This is not as good as updating on the loop with dt, but its a nice example
            auto [transform] = get_components<TransformComponent>();
            if (!transform)
            {
                LOG_WARNING("Missing transform");
                return;
            }

            const ivec2 mouse_dir = {e.x_direction, e.y_direction};

            // Rotate
            pitch += -mouse_dir.y * mouse_sensitivity;
            yaw += -mouse_dir.x * mouse_sensitivity;
        }

        vec3 get_right_dir() const
        {
            vec3 right = vec3(0.0f);
            right.x = cos(yaw);
            right.y = 0;
            right.z = -sin(yaw);

            return right;
        }

        vec3 get_forward_dir() const
        {
            vec3 forward = vec3(0.0f);
            forward.x = cos(-pitch) * sin(yaw);
            forward.y = sin(-pitch);
            forward.z = cos(-pitch) * cos(yaw);

            return forward;
        }
};

extern "C" ScriptableEntity* create_script() { return new PlayerController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
