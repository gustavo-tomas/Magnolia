#include "camera/camera.hpp"

#include "core/types.hpp"

namespace mag
{
    Camera::Camera(const vec3& position, const vec3& rotation, const f32 fov, const vec2& size, const f32 near,
                   const f32 far)
        : position(position), rotation(rotation), fov(fov), aspect_ratio(size.x / size.y), near(near), far(far)
    {
        calculate_view();
        calculate_projection();
    }

    void Camera::calculate_view()
    {
        const quat pitch_rotation = angleAxis(radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
        const quat yaw_rotation = angleAxis(radians(rotation.y), vec3(0.0f, -1.0f, 0.0f));

        this->rotation_mat = toMat4(yaw_rotation) * toMat4(pitch_rotation);
        const mat4 translation = translate(mat4(1.0f), position);

        this->view = inverse(translation * rotation_mat);
    }

    void Camera::calculate_projection()
    {
        this->projection = perspective(radians(fov), aspect_ratio, near, far);
        this->projection[1][1] *= -1;
    }

    void Camera::set_position(const vec3& position)
    {
        this->position = position;
        calculate_view();
    }

    void Camera::set_rotation(const vec3& rotation)
    {
        // Constrain rotation between [-180, 180)
        for (u32 i = 0; i < 3; i++)
        {
            this->rotation[i] = fmod(rotation[i] + 180.0f, 360.0f);
            if (this->rotation[i] < 0.0f) this->rotation[i] += 360.0f;
            this->rotation[i] -= 180.0f;
        }

        calculate_view();
    }

    void Camera::set_aspect_ratio(const vec2& size)
    {
        this->aspect_ratio = size.x / size.y;
        calculate_projection();
    }

    void Camera::set_fov(const f32 fov)
    {
        this->fov = fov;
        calculate_projection();
    }

    void Camera::set_near_far(const vec2& near_far)
    {
        this->near = near_far.x;
        this->far = near_far.y;
        calculate_projection();
    }
};  // namespace mag
