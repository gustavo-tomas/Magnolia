#include "magnolia/camera/camera.hpp"

#include "magnolia/math/functions.hpp"

namespace mag
{
    void Camera::calculate_view()
    {
        const math::mat4 translation = translate(math::mat4(1.0F), position);

        view = inverse(translation * rotation_mat);
    }

    void Camera::set_position(const math::vec3& camera_position)
    {
        this->position = camera_position;
        is_view_outdated = true;
    }

    void Camera::set_rotation(const math::quat& rotation)
    {
        rotation_mat = to_mat4(math::normalize(rotation));
        is_view_outdated = true;
    }

    void Camera::set_viewport_size(const math::vec2& size)
    {
        aspect_ratio = size.x / size.y;
        is_projection_outdated = true;
    }

    void Camera::set_near_far(const math::vec2& near_far)
    {
        near = near_far.x;
        far = near_far.y;
        is_projection_outdated = true;
    }

    const math::mat4& Camera::get_view()
    {
        if (is_view_outdated)
        {
            calculate_view();
            is_view_outdated = false;
        }

        return view;
    }

    const math::mat4& Camera::get_projection()
    {
        if (is_projection_outdated)
        {
            calculate_projection();
            is_projection_outdated = false;
        }

        return projection;
    }

    const math::vec3& Camera::get_position() const { return position; }
    math::quat Camera::get_rotation() const { return to_quat(rotation_mat); }
    const math::mat4& Camera::get_rotation_mat() const { return rotation_mat; }

    f32 Camera::get_near() const { return near; }
    f32 Camera::get_far() const { return far; }
    f32 Camera::get_aspect_ratio() const { return aspect_ratio; }

    math::vec3 Camera::get_side() const { return math::vec3(rotation_mat[0]); }
    math::vec3 Camera::get_up() const { return math::vec3(rotation_mat[1]); }
    math::vec3 Camera::get_forward() const { return math::vec3(rotation_mat[2]); }
    math::vec2 Camera::get_near_far() const { return {near, far}; }

    // PerspectiveCamera
    // -----------------------------------------------------------------------------------------------------------------

    PerspectiveCamera::PerspectiveCamera(const PerspectiveCameraDesc& camera_desc)
    {
        set_position(camera_desc.position);
        set_rotation(camera_desc.rotation);
        set_near_far({camera_desc.near, camera_desc.far});
        set_viewport_size(camera_desc.viewport_size);
        set_fov(camera_desc.fov);
    }

    PerspectiveCamera::~PerspectiveCamera() = default;

    void PerspectiveCamera::set_fov(const f32 camera_fov)
    {
        this->fov = camera_fov;
        is_projection_outdated = true;
    }

    void PerspectiveCamera::calculate_projection() { projection = math::perspective(fov, aspect_ratio, near, far); }

    f32 PerspectiveCamera::get_fov() const { return fov; }

    // OrthographicCamera
    // -----------------------------------------------------------------------------------------------------------------

    OrthographicCamera::OrthographicCamera(const OrthographicCameraDesc& camera_desc)
    {
        set_position(camera_desc.position);
        set_rotation(camera_desc.rotation);
        set_near_far({camera_desc.near, camera_desc.far});
        set_viewport_size(camera_desc.viewport_size);
        set_size(camera_desc.size);
    }

    OrthographicCamera::~OrthographicCamera() = default;

    void OrthographicCamera::set_size(const f32 camera_size)
    {
        this->size = camera_size;
        is_projection_outdated = true;
    }

    void OrthographicCamera::calculate_projection()
    {
        const f32 left = -size * aspect_ratio * 0.5F;
        const f32 right = size * aspect_ratio * 0.5F;
        const f32 bottom = -size * 0.5F;
        const f32 top = size * 0.5F;

        projection = math::ortho(left, right, bottom, top, near, far);
    }

    f32 OrthographicCamera::get_size() const { return size; }
};  // namespace mag
