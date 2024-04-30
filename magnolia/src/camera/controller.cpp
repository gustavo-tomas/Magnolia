#include "camera/controller.hpp"

#include "core/application.hpp"
#include "core/window.hpp"

namespace mag
{
    void RuntimeController::update(const f32 dt)
    {
        vec3 direction(0.0f);
        const f32 velocity = 350.0f;

        auto& window = get_application().get_window();
        if (window.is_key_down(SDLK_a)) direction.x -= 1.0f;
        if (window.is_key_down(SDLK_d)) direction.x += 1.0f;
        if (window.is_key_down(SDLK_w)) direction.z -= 1.0f;
        if (window.is_key_down(SDLK_s)) direction.z += 1.0f;
        if (window.is_key_down(SDLK_SPACE)) direction.y += 1.0f;
        if (window.is_key_down(SDLK_LCTRL)) direction.y -= 1.0f;

        // Prevent nan values
        if (length(direction) > 0.0f) direction = normalize(direction) * dt;

        const mat4& camera_rot = camera->get_rotation_mat();
        const vec3& camera_position = camera->get_position() + vec3(camera_rot * vec4(direction * velocity, 0.0f));
        camera->set_position(camera_position);
    }

    void RuntimeController::on_mouse_move(const ivec2& mouse_dir)
    {
        const vec3 new_rot = this->camera->get_rotation() + (vec3(-mouse_dir.y, mouse_dir.x, 0.0f) / 10.0f);
        this->camera->set_rotation(new_rot);
    }

    void EditorController::on_event(Event& e)
    {
        if (auto event = dynamic_cast<MouseMoveEvent*>(&e))
        {
            on_mouse_move(*event);
        }

        else if (auto event = dynamic_cast<MouseScrollEvent*>(&e))
        {
            on_mouse_scroll(*event);
        }
    }

    void EditorController::on_mouse_move(MouseMoveEvent& e)
    {
        auto& window = get_application().get_window();

        const ivec2 mouse_dir = {e.x_direction, e.y_direction};

        // Rotate
        if (window.is_button_down(SDL_BUTTON_MIDDLE))
        {
            const vec3 new_rot = this->camera->get_rotation() + (vec3(-mouse_dir.y, mouse_dir.x, 0.0f) / 10.0f);
            this->camera->set_rotation(new_rot);
        }

        // Translate
        else if (window.is_key_down(SDLK_LSHIFT))
        {
            const vec3 side = this->camera->get_side();
            const vec3 up = this->camera->get_up();

            vec3 camera_position = camera->get_position();
            camera_position += up * static_cast<f32>(mouse_dir.y) * 0.25f;
            camera_position += side * static_cast<f32>(-mouse_dir.x) * 0.25f;

            this->camera->set_position(camera_position);
        }
    }

    void EditorController::on_mouse_scroll(MouseScrollEvent& e)
    {
        const ivec2 mouse_scroll = {e.x_offset, e.y_offset};

        // We dont change the camera fov, just the position (avoid fov distortions)
        const vec3 forward = this->camera->get_forward();

        vec3 camera_position = camera->get_position();
        camera_position += forward * static_cast<f32>(-mouse_scroll.y) * 25.0f;

        this->camera->set_position(camera_position);
    }
};  // namespace mag
