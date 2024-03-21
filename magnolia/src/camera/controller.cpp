#include "camera/controller.hpp"

#include "core/logger.hpp"

namespace mag
{
    void Controller::initialize(Camera* camera, Window* window)
    {
        this->camera = camera;
        this->window = window;
    }

    void Controller::shutdown()
    {
        this->camera = nullptr;
        this->window = nullptr;
    }

    void Controller::update(const f32 dt)
    {
        vec3 direction(0.0f);
        const f32 velocity = 350.0f;

        if (window->is_key_down(SDLK_a)) direction.x -= 1.0f;
        if (window->is_key_down(SDLK_d)) direction.x += 1.0f;
        if (window->is_key_down(SDLK_w)) direction.z -= 1.0f;
        if (window->is_key_down(SDLK_s)) direction.z += 1.0f;
        if (window->is_key_down(SDLK_SPACE)) direction.y += 1.0f;
        if (window->is_key_down(SDLK_LCTRL)) direction.y -= 1.0f;

        // Prevent nan values
        if (length(direction) > 0.0f) direction = normalize(direction) * dt;

        const mat4& camera_rot = camera->get_rotation_mat();
        const vec3& camera_position = camera->get_position() + vec3(camera_rot * vec4(direction * velocity, 0.0f));
        camera->set_position(camera_position);
    }

    void Controller::on_mouse_move(const ivec2& mouse_dir)
    {
        const vec3 new_rot = this->camera->get_rotation() + (vec3(-mouse_dir.y, mouse_dir.x, 0.0f) / 10.0f);
        this->camera->set_rotation(new_rot);
    }
};  // namespace mag
