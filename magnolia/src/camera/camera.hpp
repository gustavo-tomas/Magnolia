#pragma once

#include "core/types.hpp"

namespace mag
{
    using namespace math;

    class Camera
    {
        public:
            void initialize(const vec3& position, const vec3& rotation, const f32 fov, const vec2& size, const f32 near,
                            const f32 far);
            void shutdown();

            void set_position(const vec3& position);
            void set_rotation(const vec3& rotation);
            void set_aspect_ratio(const vec2& size);

            const mat4& get_view() const { return view; };
            const mat4& get_projection() const { return projection; };
            const vec3& get_position() const { return position; };
            const vec3& get_rotation() const { return rotation; };
            const mat4& get_rotation_mat() const { return rotation_mat; };

        private:
            void calculate_view();
            void calculate_projection();

            mat4 view, projection, rotation_mat;
            vec3 position, rotation;
            f32 fov, aspect_ratio, near, far;
    };
};  // namespace mag
