#include <magnolia/platform/window.hpp>
#include <magnolia/resources/model.hpp>

#include "common.hpp"

using namespace game;

class CameraController : public ScriptableEntity
{
    private:
        const f32 mouse_sensitivity = 0.2f;
        const f32 speed = 50.0f;
        f32 pitch = 0.0f;
        f32 yaw = math::radians(-90.0f);
        vec3 initial_position = vec3(-900.0f, 450.0f, -175.0f);

    public:
        void on_create() override
        {
            auto [camera_c] = get_components<PerspectiveCameraComponent>();
            if (camera_c == nullptr)
            {
                LOG_WARNING("Missing camera");
                return;
            }

            mag::quat initial_rotation = mag::vec3(pitch, yaw, 0.0f);
            initial_rotation = math::normalize(initial_rotation);

            camera_c->camera.set_rotation(initial_rotation);
            camera_c->camera.set_position(initial_position);

            LOG_SUCCESS("Created CameraController");
        }

        void on_destroy() override { LOG_SUCCESS("Destroyed CameraController"); }

        void on_update(const f32 dt) override
        {
            auto [camera_c] = get_components<PerspectiveCameraComponent>();
            if (camera_c == nullptr)
            {
                LOG_WARNING("Missing camera");
                return;
            }

            // Handle mouse inputs first
            if (mag::window::is_mouse_captured())
            {
                const mag::math::ivec2 mouse_position = mag::window::get_mouse_position();
                const mag::math::ivec2 window_center = mag::window::get_window_center();
                const mag::math::vec2 mouse_delta = window_center - mouse_position;

                // Rotate
                pitch += mouse_delta.y * mouse_sensitivity * dt;
                yaw += mouse_delta.x * mouse_sensitivity * dt;

                mag::window::set_mouse_position(window_center.x, window_center.y);
            }

            vec3 position = camera_c->camera.get_position();
            mag::quat rotation = mag::vec3(pitch, yaw, 0.0f);

            rotation = math::normalize(rotation);

            // Calculate desired movement direction
            const mat4 rotation_mat = toMat4(rotation);
            const mag::vec3& right = rotation_mat[0];
            const mag::vec3& up = rotation_mat[1];
            const mag::vec3& forward = rotation_mat[2];

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
            if (mag::window::is_key_down(mag::Key::Space))
            {
                input_direction += up;
            }
            if (mag::window::is_key_down(mag::Key::Lctrl))
            {
                input_direction -= up;
            }

            // Prevent nan values
            if (length(input_direction) > 0.0f)
            {
                input_direction = normalize(input_direction);
                position += input_direction * speed * dt;
            }

            // Update the camera transform
            camera_c->camera.set_rotation(rotation);
            camera_c->camera.set_position(position);
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
