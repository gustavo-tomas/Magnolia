#include "magnolia/camera/camera.hpp"

#include "magnolia/camera/frustum.hpp"
#include "magnolia/math/functions.hpp"

namespace mag
{
    void Camera::calculate_frustum() { frustum = Frustum(projection * view); }

    void Camera::calculate_view()
    {
        const mat4 translation = translate(mat4(1.0F), position);

        view = inverse(translation * rotation_mat);

        calculate_frustum();
    }

    void Camera::set_position(const vec3& camera_position)
    {
        this->position = camera_position;
        calculate_view();
    }

    void Camera::set_rotation(const quat& rotation)
    {
        rotation_mat = to_mat4(math::normalize(rotation));
        calculate_view();
    }

    void Camera::set_viewport_size(const vec2& size)
    {
        aspect_ratio = size.x / size.y;
        calculate_projection();
    }

    void Camera::set_near_far(const vec2& near_far)
    {
        near = near_far.x;
        far = near_far.y;
        calculate_projection();
    }

    b8 Camera::is_aabb_visible(const BoundingBox& aabb) const { return frustum.is_aabb_visible(aabb); }

    const mat4& Camera::get_view() const { return view; }
    const mat4& Camera::get_projection() const { return projection; }
    const vec3& Camera::get_position() const { return position; }
    quat Camera::get_rotation() const { return to_quat(rotation_mat); }
    const mat4& Camera::get_rotation_mat() const { return rotation_mat; }
    const Frustum& Camera::get_frustum() const { return frustum; }

    f32 Camera::get_near() const { return near; }
    f32 Camera::get_far() const { return far; }
    f32 Camera::get_aspect_ratio() const { return aspect_ratio; }

    vec3 Camera::get_side() const { return vec3(rotation_mat[0]); }
    vec3 Camera::get_up() const { return vec3(rotation_mat[1]); }
    vec3 Camera::get_forward() const { return vec3(rotation_mat[2]); }
    vec2 Camera::get_near_far() const { return {near, far}; }

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
        calculate_projection();
    }

    void PerspectiveCamera::calculate_projection()
    {
        projection = perspective(fov, aspect_ratio, near, far);
        calculate_frustum();
    }

    const f32& PerspectiveCamera::get_fov() const { return fov; }

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
        calculate_projection();
    }

    void OrthographicCamera::calculate_projection()
    {
        const f32 left = -size * aspect_ratio * 0.5F;
        const f32 right = size * aspect_ratio * 0.5F;
        const f32 bottom = -size * 0.5F;
        const f32 top = size * 0.5F;

        projection = ortho(left, right, bottom, top, near, far);

        calculate_frustum();
    }

    f32 OrthographicCamera::get_size() const { return size; }
};  // namespace mag
