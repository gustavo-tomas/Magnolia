#pragma once

#include "core/types.hpp"

namespace mag
{
    using namespace math;

    struct BoundingBox;

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

            b8 is_aabb_visible(const BoundingBox& aabb) const;

        private:
            void calculate_view();
            void calculate_projection();

            mat4 view, projection, rotation_mat;
            vec3 position, rotation;
            f32 fov, aspect_ratio, near, far;
    };

    // Taken from: https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
    class Frustum
    {
        public:
            Frustum() = default;

            // m = ProjectionMatrix * ViewMatrix
            Frustum(mat4 m);

            // https://iquilezles.org/articles/frustumcorrect/
            b8 is_aabb_visible(const BoundingBox& aabb) const;

        private:
            enum Planes
            {
                Left = 0,
                Right,
                Bottom,
                Top,
                Near,
                Far,
                Count,
                Combinations = Count * (Count - 1) / 2
            };

            template <Planes i, Planes j>
            struct ij2k
            {
                    enum
                    {
                        k = i * (9 - i) / 2 + j - 1
                    };
            };

            template <Planes a, Planes b, Planes c>
            vec3 intersection(const vec3* crosses) const;

            vec4 planes[Count];
            vec3 points[8];
    };
};  // namespace mag
