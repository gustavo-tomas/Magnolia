#include "camera/camera.hpp"

namespace mag
{
    void Camera::initialize(const vec3& position, const vec3& rotation, const f32 fov, const vec2& size, const f32 near,
                            const f32 far)
    {
        this->position = position;
        this->rotation = rotation;
        this->fov = fov;
        this->aspect_ratio = size.x / size.y;
        this->near = near;
        this->far = far;

        calculate_view();
        calculate_projection();
    }

    void Camera::shutdown() {}

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
        this->rotation = rotation;
        calculate_view();
    }

    void Camera::set_aspect_ratio(const vec2& size)
    {
        this->aspect_ratio = size.x / size.y;
        calculate_projection();
    }
};  // namespace mag
