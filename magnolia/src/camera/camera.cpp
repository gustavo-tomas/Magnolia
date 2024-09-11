#include "camera/camera.hpp"

#include "core/types.hpp"
#include "resources/model.hpp"

namespace mag
{
    Camera::Camera(const vec3& position, const vec3& rotation, const f32 fov, const f32 aspect_ratio, const f32 near,
                   const f32 far)
        : position(position), rotation(rotation), fov(fov), aspect_ratio(aspect_ratio), near(near), far(far)
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

    b8 Camera::is_aabb_visible(const BoundingBox& aabb) const
    {
        Frustum camera_frustum(this->projection * this->view);

        return camera_frustum.is_aabb_visible(aabb);
    }

    Frustum::Frustum(mat4 m)
    {
        m = transpose(m);
        planes[Left] = m[3] + m[0];
        planes[Right] = m[3] - m[0];
        planes[Bottom] = m[3] + m[1];
        planes[Top] = m[3] - m[1];
        planes[Near] = m[3] + m[2];
        planes[Far] = m[3] - m[2];

        vec3 crosses[Combinations] = {
            cross(vec3(planes[Left]), vec3(planes[Right])),  cross(vec3(planes[Left]), vec3(planes[Bottom])),
            cross(vec3(planes[Left]), vec3(planes[Top])),    cross(vec3(planes[Left]), vec3(planes[Near])),
            cross(vec3(planes[Left]), vec3(planes[Far])),    cross(vec3(planes[Right]), vec3(planes[Bottom])),
            cross(vec3(planes[Right]), vec3(planes[Top])),   cross(vec3(planes[Right]), vec3(planes[Near])),
            cross(vec3(planes[Right]), vec3(planes[Far])),   cross(vec3(planes[Bottom]), vec3(planes[Top])),
            cross(vec3(planes[Bottom]), vec3(planes[Near])), cross(vec3(planes[Bottom]), vec3(planes[Far])),
            cross(vec3(planes[Top]), vec3(planes[Near])),    cross(vec3(planes[Top]), vec3(planes[Far])),
            cross(vec3(planes[Near]), vec3(planes[Far]))};

        points[0] = intersection<Left, Bottom, Near>(crosses);
        points[1] = intersection<Left, Top, Near>(crosses);
        points[2] = intersection<Right, Bottom, Near>(crosses);
        points[3] = intersection<Right, Top, Near>(crosses);
        points[4] = intersection<Left, Bottom, Far>(crosses);
        points[5] = intersection<Left, Top, Far>(crosses);
        points[6] = intersection<Right, Bottom, Far>(crosses);
        points[7] = intersection<Right, Top, Far>(crosses);
    }

    b8 Frustum::is_aabb_visible(const BoundingBox& aabb) const
    {
        const vec3& minp = aabb.min;
        const vec3& maxp = aabb.max;

        // check box outside/inside of frustum
        for (u32 i = 0; i < Count; i++)
        {
            if ((dot(planes[i], vec4(minp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(maxp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(minp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(maxp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(minp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(maxp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(minp.x, maxp.y, maxp.z, 1.0f)) < 0.0) &&
                (dot(planes[i], vec4(maxp.x, maxp.y, maxp.z, 1.0f)) < 0.0))
            {
                return false;
            }
        }

        // check frustum outside/inside box
        u32 out;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((points[i].x > maxp.x) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((points[i].x < minp.x) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((points[i].y > maxp.y) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((points[i].y < minp.y) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((points[i].z > maxp.z) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((points[i].z < minp.z) ? 1 : 0);
        if (out == 8) return false;

        return true;
    }

    template <Frustum::Planes a, Frustum::Planes b, Frustum::Planes c>
    vec3 Frustum::intersection(const vec3* crosses) const
    {
        const f32 D = dot(vec3(planes[a]), crosses[ij2k<b, c>::k]);
        const vec3 res = mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
                         vec3(planes[a].w, planes[b].w, planes[c].w);

        return res * (-1.0f / D);
    }
};  // namespace mag
