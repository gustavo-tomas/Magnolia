#include "common.hpp"
#include "magnolia/core/logger.hpp"

using namespace mag;

class CameraController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created CameraController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed CameraController"); }

        virtual void on_update(const f32 dt) override
        {
            auto [transform, camera_c] = get_components<TransformComponent, CameraComponent>();
            if (!transform)
            {
                LOG_WARNING("Missing transform/camera");
                return;
            }

            // Handle mouse inputs first
            if (window::is_mouse_captured())
            {
                const math::ivec2 mouse_position = window::get_mouse_position();
                const math::ivec2 window_center = window::get_window_center();
                math::vec2 mouse_delta = window_center - mouse_position;

                // Rotate

                const f32 mouse_sensitivity = 0.002f;
                mouse_delta.x *= mouse_sensitivity;
                mouse_delta.y *= mouse_sensitivity;

                transform->rotation += vec3(mouse_delta.y, mouse_delta.x, 0.0f);

                window::set_mouse_position(window_center.x, window_center.y);
            }

            const mat4 rotation_mat = calculate_rotation_mat(transform->rotation);
            const vec3 side = rotation_mat[0];
            const vec3 up = rotation_mat[1];
            const vec3 forward = rotation_mat[2];

            vec3 direction(0.0f);
            const f32 speed = 50.0f;

            if (window::is_key_down(Key::a)) direction -= side;
            if (window::is_key_down(Key::d)) direction += side;
            if (window::is_key_down(Key::w)) direction -= forward;
            if (window::is_key_down(Key::s)) direction += forward;
            if (window::is_key_down(Key::Space)) direction += up;
            if (window::is_key_down(Key::Lctrl)) direction -= up;

            // Prevent nan values
            if (length(direction) > 0.0f)
            {
                direction = normalize(direction) * dt;
                transform->translation += direction * speed;
            }

            // Update the camera transform
            camera_c->camera.set_rotation(transform->rotation);
            camera_c->camera.set_position(transform->translation);
        }

        virtual void on_event(const Event& e) override
        {
            dispatch_event<MousePressEvent>(e, BIND_FN(CameraController::on_mouse_click));
            dispatch_event<KeyPressEvent>(e, BIND_FN(CameraController::on_key_press));
        }

        void on_key_press(const KeyPressEvent& e)
        {
            // Swap scenes
            if (e.key == Key::Tab)
            {
                set_active_scene("test_game/assets/scenes/Main.mag.json");
            }
        }

        void on_mouse_click(const MousePressEvent& e)
        {
            // Capture/Release the cursor
            if (e.button == Button::Right)
            {
                const b8 capture = !window::is_mouse_captured();

                window::set_capture_mouse(capture);

                // Keep mouse button centered
                if (capture)
                {
                    const math::ivec2 window_center = window::get_window_center();
                    window::set_mouse_position(window_center.x, window_center.y);
                }
            }
        }
};

extern "C" ScriptableEntity* create_script() { return new CameraController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
