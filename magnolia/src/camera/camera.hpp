#pragma once

#include "core/types.hpp"

namespace mag
{
    using namespace math;

    class Camera
    {
        public:
            Camera(const vec3& position, const vec3& rotation, const f32 fov, const f32 aspect_ratio, const f32 near,
                   const f32 far);
            ~Camera() = default;

            void set_position(const vec3& position);
            void set_rotation(const vec3& rotation);
            void set_aspect_ratio(const vec2& size);
            void set_fov(const f32 fov);
            void set_near_far(const vec2& near_far);

            const f32& get_fov() const { return fov; };
            const mat4& get_view() const { return view; };
            const mat4& get_projection() const { return projection; };
            const vec3& get_position() const { return position; };
            const vec3& get_rotation() const { return rotation; };
            const mat4& get_rotation_mat() const { return rotation_mat; };

            f32 get_near() const { return near; };
            f32 get_far() const { return far; };
            f32 get_aspect_ratio() const { return aspect_ratio; };

            vec3 get_side() const { return rotation_mat[0]; };
            vec3 get_up() const { return rotation_mat[1]; };
            vec3 get_forward() const { return rotation_mat[2]; };
            vec2 get_near_far() const { return {near, far}; };

        private:
            void calculate_view();
            void calculate_projection();

            mat4 view, projection, rotation_mat;
            vec3 position, rotation;
            f32 fov, aspect_ratio, near, far;
    };
};  // namespace mag
