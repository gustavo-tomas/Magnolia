#include "magnolia/camera/camera.hpp"

#include "magnolia/camera/frustum.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    void Camera::calculate_frustum() { this->frustum = Frustum(this->projection * this->view); }

    void Camera::calculate_view()
    {
        this->rotation_mat = calculate_rotation_mat(this->rotation);
        const mat4 translation = translate(mat4(1.0f), this->position);

        this->view = inverse(translation * this->rotation_mat);

        calculate_frustum();
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
            this->rotation[i] = fmod(rotation[i] + math::pi<f32>(), math::two_pi<f32>());
            if (this->rotation[i] < 0.0f)
            {
                this->rotation[i] += math::two_pi<f32>();
            }
            this->rotation[i] -= math::pi<f32>();
        }

        calculate_view();
    }

    void Camera::set_viewport_size(const vec2& size)
    {
        this->aspect_ratio = size.x / size.y;
        calculate_projection();
    }

    void Camera::set_near_far(const vec2& near_far)
    {
        this->near = near_far.x;
        this->far = near_far.y;
        calculate_projection();
    }

    b8 Camera::is_aabb_visible(const BoundingBox& aabb) const { return this->frustum.is_aabb_visible(aabb); }

    const mat4& Camera::get_view() const { return this->view; }
    const mat4& Camera::get_projection() const { return this->projection; }
    const vec3& Camera::get_position() const { return this->position; }
    const vec3& Camera::get_rotation() const { return this->rotation; }
    const mat4& Camera::get_rotation_mat() const { return this->rotation_mat; }
    const Frustum& Camera::get_frustum() const { return this->frustum; }

    f32 Camera::get_near() const { return this->near; }
    f32 Camera::get_far() const { return this->far; }
    f32 Camera::get_aspect_ratio() const { return this->aspect_ratio; }

    vec3 Camera::get_side() const { return this->rotation_mat[0]; }
    vec3 Camera::get_up() const { return this->rotation_mat[1]; }
    vec3 Camera::get_forward() const { return this->rotation_mat[2]; }
    vec2 Camera::get_near_far() const { return {this->near, this->far}; }

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

    void PerspectiveCamera::set_fov(const f32 fov)
    {
        this->fov = fov;
        calculate_projection();
    }

    void PerspectiveCamera::calculate_projection()
    {
        this->projection = perspective(this->fov, this->aspect_ratio, this->near, this->far);
        calculate_frustum();
    }

    const f32& PerspectiveCamera::get_fov() const { return this->fov; }

    // OrthographicCamera
    // -----------------------------------------------------------------------------------------------------------------

    OrthographicCamera::OrthographicCamera(const OrhographicCameraDesc& camera_desc)
    {
        set_position(camera_desc.position);
        set_rotation(camera_desc.rotation);
        set_near_far({camera_desc.near, camera_desc.far});
        set_viewport_size(camera_desc.viewport_size);
        set_size(camera_desc.size);
    }

    OrthographicCamera::~OrthographicCamera() = default;

    void OrthographicCamera::set_size(const f32 size)
    {
        this->size = size;
        calculate_projection();
    }

    void OrthographicCamera::calculate_projection()
    {
        const f32 left = -size * aspect_ratio * 0.5f;
        const f32 right = size * aspect_ratio * 0.5f;
        const f32 bottom = -size * 0.5f;
        const f32 top = size * 0.5f;

        this->projection = ortho(left, right, bottom, top, near, far);

        calculate_frustum();
    }

    f32 OrthographicCamera::get_size() const { return this->size; }
};  // namespace mag
