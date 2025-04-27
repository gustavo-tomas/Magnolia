#include <ecs/components.hpp>
#include <magnolia.hpp>
#include <physics/physics.hpp>
#include <resources/model.hpp>

#include "common.hpp"

using namespace mag;

class PlayerController : public ScriptableEntity
{
    private:
        f32 hp = 100.0f;

    public:
        virtual void on_create() override { LOG_SUCCESS("Created PlayerController"); }

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

            Application& app = get_application();
            Window& window = app.get_window();

            if (window.is_button_pressed(Button::Left))
            {
                fire_bullet(*transform);
            }
        }

        void handle_movement(const f32 dt)
        {
            Application& app = get_application();
            Window& window = app.get_window();

            auto [transform, camera_c] = get_components<TransformComponent, CameraComponent>();
            if (!transform)
            {
                LOG_WARNING("Missing transform/camera");
                return;
            }

            const mat4 rotation_mat = calculate_rotation_mat(transform->rotation);
            const vec3 side = rotation_mat[0];
            const vec3 up = rotation_mat[1];
            const vec3 forward = rotation_mat[2];

            vec3 direction(0.0f);
            const f32 speed = 50.0f;

            if (window.is_key_down(Key::a)) direction -= side;
            if (window.is_key_down(Key::d)) direction += side;
            if (window.is_key_down(Key::w)) direction -= forward;
            if (window.is_key_down(Key::s)) direction += forward;
            if (window.is_key_down(Key::Space)) direction += up;
            if (window.is_key_down(Key::Lctrl)) direction -= up;

            // Prevent nan values
            if (length(direction) > 0.0f)
            {
                direction = normalize(direction) * dt;
                transform->translation += direction * speed;
            }

            // Update the camera transform
            const mat4 cam_rotation_mat = calculate_rotation_mat(transform->rotation);
            const vec3 cam_forward = math::normalize(cam_rotation_mat[2]);

            camera_c->camera.set_rotation(transform->rotation);
            camera_c->camera.set_position(transform->translation + cam_forward * vec3(50));
        }

        void fire_bullet(const TransformComponent& transform)
        {
            PhysicsWorld& physics = get_physics_world();

            // Create a bullet
            const u32 bullet_id = create_entity();

            TransformComponent* bullet_transform = new TransformComponent(transform);
            bullet_transform->scale = vec3(100.0f);

            const ref<Model> model =
                resource::get_model("test_game/assets/models/hammer/native/wooden_hammer_01.model.json");

            auto* model_c = new ModelComponent(model);
            auto* rigid_body = new RigidBodyComponent(10.0f);
            auto* collider = new BoxColliderComponent(vec3(10.0f));

            add_component_to_entity(bullet_id, bullet_transform);
            add_component_to_entity(bullet_id, model_c);
            add_component_to_entity(bullet_id, rigid_body);
            add_component_to_entity(bullet_id, collider);

            const vec3& forward_dir = get_forward_dir(*bullet_transform);
            physics.apply_impulse(rigid_body->collision_object, -forward_dir * 1000.0f);
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
                auto& app = get_application();
                auto& window = app.get_window();

                window.set_capture_mouse(!window.is_mouse_captured());
            }
        }

        void on_mouse_move(const MouseMoveEvent& e)
        {
            // This is not as good as updating on the loop with dt, but its a nice example
            auto [transform] = get_components<TransformComponent>();
            if (!transform)
            {
                LOG_WARNING("Missing transform/camera");
                return;
            }

            const ivec2 mouse_dir = {e.x_direction, e.y_direction};

            // Rotate
            transform->rotation += vec3(-mouse_dir.y, -mouse_dir.x, 0.0f) / 250.0f;
        }

        vec3 get_side_dir(const TransformComponent& transform) const
        {
            const mat4 rotation_mat = calculate_rotation_mat(transform.rotation);
            return rotation_mat[0];
        }

        vec3 get_up_dir(const TransformComponent& transform) const
        {
            const mat4 rotation_mat = calculate_rotation_mat(transform.rotation);
            return rotation_mat[1];
        }

        vec3 get_forward_dir(const TransformComponent& transform) const
        {
            const mat4 rotation_mat = calculate_rotation_mat(transform.rotation);
            return rotation_mat[2];
        }
};

extern "C" ScriptableEntity* create_script() { return new PlayerController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
