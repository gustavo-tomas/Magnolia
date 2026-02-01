#include <magnolia/platform/window.hpp>
#include <magnolia/resources/model.hpp>

#include "common.hpp"

using namespace game;

class CameraController : public ScriptableEntity
{
    public:
        void on_create() override { LOG_SUCCESS("Created CameraController"); }

        void on_destroy() override { LOG_SUCCESS("Destroyed CameraController"); }

        void on_update(const f32 dt) override
        {
            auto [camera_c] = get_components<PerspectiveCameraComponent>();
            if (camera_c == nullptr)
            {
                LOG_WARNING("Missing transform/camera");
                return;
            }

            vec3 translation = camera_c->camera.get_position();
            vec3 rotation = camera_c->camera.get_rotation();

            // Handle mouse inputs first
            if (mag::window::is_mouse_captured())
            {
                const mag::math::ivec2 mouse_position = mag::window::get_mouse_position();
                const mag::math::ivec2 window_center = mag::window::get_window_center();
                mag::math::vec2 mouse_delta = window_center - mouse_position;

                // Rotate

                const f32 mouse_sensitivity = 0.002f;
                mouse_delta.x *= mouse_sensitivity;
                mouse_delta.y *= mouse_sensitivity;

                rotation += vec3(mouse_delta.y, mouse_delta.x, 0.0f);

                mag::window::set_mouse_position(window_center.x, window_center.y);
            }

            const mat4 rotation_mat = mag::calculate_rotation_mat(rotation);
            const vec3 side = rotation_mat[0];
            const vec3 up = rotation_mat[1];
            const vec3 forward = rotation_mat[2];

            vec3 direction(0.0f);
            const f32 speed = 50.0f;

            if (mag::window::is_key_down(mag::Key::a))
            {
                direction -= side;
            }
            if (mag::window::is_key_down(mag::Key::d))
            {
                direction += side;
            }
            if (mag::window::is_key_down(mag::Key::w))
            {
                direction -= forward;
            }
            if (mag::window::is_key_down(mag::Key::s))
            {
                direction += forward;
            }
            if (mag::window::is_key_down(mag::Key::Space))
            {
                direction += up;
            }
            if (mag::window::is_key_down(mag::Key::Lctrl))
            {
                direction -= up;
            }

            // Prevent nan values
            if (length(direction) > 0.0f)
            {
                direction = normalize(direction) * dt;
                translation += direction * speed;
            }

            // Update the camera transform
            camera_c->camera.set_rotation(rotation);
            camera_c->camera.set_position(translation);
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
                set_active_scene("test_game/assets/scenes/Main.mag.json");
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
};

extern "C" ScriptableEntity* create_script() { return new CameraController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
