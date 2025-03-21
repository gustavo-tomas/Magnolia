#include "camera_controller.hpp"

#include "camera/camera.hpp"
#include "core/application.hpp"
#include "core/event.hpp"
#include "core/window.hpp"

namespace sprout
{
    EditorCameraController::EditorCameraController(Camera& camera) : camera(camera) {}

    EditorCameraController::~EditorCameraController() = default;

    void EditorCameraController::on_update(const f32 dt) { (void)dt; }

    void EditorCameraController::on_event(const Event& e)
    {
        dispatch_event<MouseMoveEvent>(e, BIND_FN(EditorCameraController::on_mouse_move));
        dispatch_event<MouseScrollEvent>(e, BIND_FN(EditorCameraController::on_mouse_scroll));
    }

    void EditorCameraController::on_mouse_move(const MouseMoveEvent& e)
    {
        auto& window = get_application().get_window();

        const ivec2 mouse_dir = {e.x_direction, e.y_direction};

        // Translate
        if (window.is_key_down(Key::Lshift) && window.is_button_down(Button::Middle))
        {
            const vec3 side = this->camera.get_side();
            const vec3 up = this->camera.get_up();

            vec3 camera_position = camera.get_position();
            camera_position += up * static_cast<f32>(mouse_dir.y) * 0.25f;
            camera_position += side * static_cast<f32>(-mouse_dir.x) * 0.25f;

            this->camera.set_position(camera_position);
        }

        // Rotate
        else if (window.is_button_down(Button::Middle))
        {
            const vec3 new_rot = this->camera.get_rotation() + (vec3(-mouse_dir.y, -mouse_dir.x, 0.0f) / 250.0f);
            this->camera.set_rotation(new_rot);
        }
    }

    void EditorCameraController::on_mouse_scroll(const MouseScrollEvent& e)
    {
        auto& window = get_application().get_window();

        // Prevent scrolling while moving
        if (window.is_button_down(Button::Middle)) return;

        const ivec2 mouse_scroll = {e.x_offset, e.y_offset};

        // We dont change the camera fov, just the position (avoid fov distortions)
        const vec3 forward = this->camera.get_forward();

        vec3 camera_position = camera.get_position();
        camera_position += forward * static_cast<f32>(-mouse_scroll.y) * 25.0f;

        this->camera.set_position(camera_position);
    }
};  // namespace sprout
