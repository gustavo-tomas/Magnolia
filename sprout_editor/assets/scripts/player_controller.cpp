#include <magnolia.hpp>
#include <math/generic.hpp>
#include <math/quat.hpp>

using namespace mag;

class PlayerController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created PlayerController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed PlayerController"); }

        virtual void on_update(const f32 dt) override
        {
            // Check for missing components
            auto [transform, rigidbody] = get_components<TransformComponent, RigidBodyComponent>();
            if (!transform || !rigidbody)
            {
                LOG_WARNING("(PlayerController) Missing player transform/rigidbody");
                return;
            }

            Application& app = get_application();
            Window& window = app.get_window();
            PhysicsWorld& physics_world = get_physics_world();

            // Move
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
            if (length(direction) <= 0.0f)
            {
                return;
            }

            direction = normalize(direction);

            const vec3 impulse = direction * speed * dt;
            physics_world.apply_impulse(rigidbody->collision_object, impulse);
        }

        virtual void on_event(const Event& e) override
        {
            // @TODO: figure out a smooth way to do rotation
            // dispatch_event<MouseMoveEvent>(e, BIND_FN(PlayerController::on_mouse_move));
            dispatch_event<MousePressEvent>(e, BIND_FN(PlayerController::on_mouse_click));
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
            TransformComponent* transform = get_component<TransformComponent>();

            // This is not as good as updating on the loop with dt, but its a nice example
            const ivec2 mouse_dir = {e.x_direction, e.y_direction};

            // Rotate
            transform->rotation += vec3(-mouse_dir.y, -mouse_dir.x, 0.0f) / 10.0f;
        }
};

extern "C" ScriptableEntity* create_script() { return new PlayerController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
