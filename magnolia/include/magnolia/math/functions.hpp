#pragma once

#include "magnolia/math/types.hpp"

namespace mag
{
    namespace math
    {
        b8 initialize();

        void shutdown();

        MAG_API inline f32 radians(const f32 angle_deg) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline f32 length(const vec3& v) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline vec3 normalize(const vec3& v) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline quat normalize(const quat& q) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 translate(const mat4& m, const vec3& position) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 translate(const vec3& position) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 scale(const mat4& m, const vec3& s) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 scale(const vec3& s) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 inverse(const mat4& m) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 transpose(const mat4& m) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline vec3 cross(const vec3& v1, const vec3& v2) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline f32 dot(const vec3& v1, const vec3& v2) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline f32 dot(const vec4& v1, const vec4& v2) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline quat angle_axis(const f32 angle, const vec3& axis) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline quat to_quat(const mat4& m) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 to_mat4(const quat& q) { MAG_ASSERT(false, "@TODO"); }

        MAG_API inline mat4 perspective(const f32 fov, const f32 aspect, const f32 near, const f32 far)
        {
            MAG_ASSERT(false, "@TODO");
        }

        MAG_API inline mat4 ortho(const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 near,
                                  const f32 far)
        {
            MAG_ASSERT(false, "@TODO");
        }

        MAG_API f32 random(const f32 begin = 0.0f, const f32 end = 1.0f);

        MAG_API i32 random(const i32 begin = 0, const i32 end = 1);

        // Calculate a rotation mat from XYZ rotation
        MAG_API mat4 calculate_rotation_mat(const vec3& rotation);

        // Get direction from angles
        MAG_API vec3 get_right_dir(const f32 yaw);

        MAG_API vec3 get_forward_dir(const f32 pitch, const f32 yaw);

        MAG_API vec3 get_up_dir(const f32 pitch, const f32 yaw);

        template <typename T>
        MAG_API inline vector3<T> min(const vector3<T>& v1, const vector3<T>& v2)
        {
            MAG_ASSERT(false, "@TODO");
        }

        template <typename T>
        MAG_API inline T min(const T v1, const T v2)
        {
            return v1 < v2 ? v1 : v2;
        }

        template <typename T>
        MAG_API inline vector3<T> max(const vector3<T>& v1, const vector3<T>& v2)
        {
            MAG_ASSERT(false, "@TODO");
        }

        template <typename T>
        MAG_API inline T max(const T v1, const T v2)
        {
            return v1 > v2 ? v1 : v2;
        }

        // String conversions

        template <typename T>
        MAG_API inline constexpr str to_string(const vector2<T>& v, const u8 precision = 3)
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                return log::get_formatted_str("(x: {0:.{2}f}, y: {1:.{2}f})", v.x, v.y, precision);
            }
            else
            {
                return log::get_formatted_str("(x: {0}, y: {1})", v.x, v.y);
            }
        }

        template <typename T>
        MAG_API inline constexpr str to_string(const vector3<T>& v, const u8 precision = 3)
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                return log::get_formatted_str("(x: {0:.{3}f}, y: {1:.{3}f}, z: {2:.{3}f})", v.x, v.y, v.z, precision);
            }
            else
            {
                return log::get_formatted_str("(x: {0}, y: {1}, z: {2})", v.x, v.y, v.z);
            }
        }
    };  // namespace math
};  // namespace mag
