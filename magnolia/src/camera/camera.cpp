#include "camera/camera.hpp"

#include "camera/frustum.hpp"
#include "math/mat.hpp"
#include "math/quat.hpp"
#include "math/types.hpp"

namespace mag
{
    struct Camera::IMPL
    {
            IMPL(const vec3& position, const vec3& rotation, const f32 fov, const f32 aspect_ratio, const f32 near,
                 const f32 far)
                : position(position), rotation(rotation), fov(fov), aspect_ratio(aspect_ratio), near(near), far(far)
            {
            }

            ~IMPL() = default;

            Frustum frustum;
            mat4 view, projection, rotation_mat;
            vec3 position, rotation;
            f32 fov, aspect_ratio, near, far;
    };

    Camera::Camera(const vec3& position, const vec3& rotation, const f32 fov, const f32 aspect_ratio, const f32 near,
                   const f32 far)
        : impl(new IMPL(position, rotation, fov, aspect_ratio, near, far))
    {
        calculate_view();
        calculate_projection();
        calculate_frustum();
    }

    Camera::Camera(const Camera& other) : impl(new IMPL(*other.impl)) {}

    Camera::~Camera() = default;

    void Camera::calculate_frustum() { impl->frustum = Frustum(impl->projection * impl->view); }

    void Camera::calculate_view()
    {
        impl->rotation_mat = calculate_rotation_mat(impl->rotation);
        const mat4 translation = translate(mat4(1.0f), impl->position);

        impl->view = inverse(translation * impl->rotation_mat);

        calculate_frustum();
    }

    void Camera::calculate_projection()
    {
        impl->projection = perspective(radians(impl->fov), impl->aspect_ratio, impl->near, impl->far);
        calculate_frustum();
    }

    void Camera::set_position(const vec3& position)
    {
        impl->position = position;
        calculate_view();
    }

    void Camera::set_rotation(const vec3& rotation)
    {
        // Constrain rotation between [-180, 180)
        for (u32 i = 0; i < 3; i++)
        {
            impl->rotation[i] = fmod(rotation[i] + 180.0f, 360.0f);
            if (impl->rotation[i] < 0.0f) impl->rotation[i] += 360.0f;
            impl->rotation[i] -= 180.0f;
        }

        calculate_view();
    }

    void Camera::set_aspect_ratio(const vec2& size)
    {
        impl->aspect_ratio = size.x / size.y;
        calculate_projection();
    }

    void Camera::set_fov(const f32 fov)
    {
        impl->fov = fov;
        calculate_projection();
    }

    void Camera::set_near_far(const vec2& near_far)
    {
        impl->near = near_far.x;
        impl->far = near_far.y;
        calculate_projection();
    }

    b8 Camera::is_aabb_visible(const BoundingBox& aabb) const { return impl->frustum.is_aabb_visible(aabb); }

    const f32& Camera::get_fov() const { return impl->fov; }
    const mat4& Camera::get_view() const { return impl->view; }
    const mat4& Camera::get_projection() const { return impl->projection; }
    const vec3& Camera::get_position() const { return impl->position; }
    const vec3& Camera::get_rotation() const { return impl->rotation; }
    const mat4& Camera::get_rotation_mat() const { return impl->rotation_mat; }
    const Frustum& Camera::get_frustum() const { return impl->frustum; }

    f32 Camera::get_near() const { return impl->near; }
    f32 Camera::get_far() const { return impl->far; }
    f32 Camera::get_aspect_ratio() const { return impl->aspect_ratio; }

    vec3 Camera::get_side() const { return impl->rotation_mat[0]; }
    vec3 Camera::get_up() const { return impl->rotation_mat[1]; }
    vec3 Camera::get_forward() const { return impl->rotation_mat[2]; }
    vec2 Camera::get_near_far() const { return {impl->near, impl->far}; }
};  // namespace mag
