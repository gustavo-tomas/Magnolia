#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    using namespace mag::math;

    class Frustum;

    class Camera
    {
        public:
            Camera(const vec3& position, const vec3& rotation, const f32 fov, const f32 aspect_ratio, const f32 near,
                   const f32 far);
            Camera(const Camera& other);
            ~Camera();

            void set_position(const vec3& position);
            void set_rotation(const vec3& rotation);
            void set_aspect_ratio(const vec2& size);
            void set_fov(const f32 fov);
            void set_near_far(const vec2& near_far);

            const f32& get_fov() const;
            const mat4& get_view() const;
            const mat4& get_projection() const;
            const vec3& get_position() const;
            const vec3& get_rotation() const;
            const mat4& get_rotation_mat() const;
            const Frustum& get_frustum() const;

            f32 get_near() const;
            f32 get_far() const;
            f32 get_aspect_ratio() const;

            vec3 get_side() const;
            vec3 get_up() const;
            vec3 get_forward() const;
            vec2 get_near_far() const;

            b8 is_aabb_visible(const BoundingBox& aabb) const;

        private:
            void calculate_view();
            void calculate_projection();
            void calculate_frustum();

            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
