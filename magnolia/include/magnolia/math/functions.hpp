#pragma once

#include <cmath>

#include "magnolia/core/logger.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    namespace math
    {
        b8 initialize();

        void shutdown();

        MAG_API f32 radians(f32 angle_deg);

        MAG_API f32 dot(const vec3& v1, const vec3& v2);

        MAG_API f32 dot(const vec4& v1, const vec4& v2);

        MAG_API f32 dot(const quat& q1, const quat& q2);

        MAG_API f32 length(const vec3& v);

        MAG_API f32 length(const quat& q);

        MAG_API vec3 normalize(const vec3& v);

        MAG_API quat normalize(const quat& q);

        MAG_API mat4 translate(const mat4& m, const vec3& position);

        MAG_API mat4 translate(const vec3& position);

        MAG_API mat4 scale(const mat4& m, const vec3& v);

        MAG_API mat4 inverse(const mat4& m);

        MAG_API mat4 transpose(const mat4& m);

        MAG_API vec3 cross(const vec3& v1, const vec3& v2);

        MAG_API quat to_quat(const mat4& m);

        MAG_API mat4 to_mat4(const quat& q);

        MAG_API mat4 perspective(f32 fov, f32 aspect, f32 near, f32 far);

        MAG_API mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);

        MAG_API f32 random(f32 begin = 0.0F, f32 end = 1.0F);

        MAG_API i32 random(i32 begin = 0, i32 end = 1);

        // Get direction from angles
        MAG_API vec3 get_right_dir(f32 yaw);

        MAG_API vec3 get_forward_dir(f32 pitch, f32 yaw);

        MAG_API vec3 get_up_dir(f32 pitch, f32 yaw);

        template <typename T>
        MAG_API inline T sin(const T x)
        {
            return std::sin(x);
        }

        template <typename T>
        MAG_API inline vector3<T> sin(const vector3<T>& v)
        {
            return {std::sin(v.x), std::sin(v.y), std::sin(v.z)};
        }

        template <typename T>
        MAG_API inline T cos(const T x)
        {
            return std::cos(x);
        }

        template <typename T>
        MAG_API inline vector3<T> cos(const vector3<T>& v)
        {
            return {std::cos(v.x), std::cos(v.y), std::cos(v.z)};
        }

        template <typename T>
        MAG_API inline T min(const T v1, const T v2)
        {
            return v1 < v2 ? v1 : v2;
        }

        // Returns the minimum value for each pair
        template <typename T>
        MAG_API inline vector3<T> min(const vector3<T>& v1, const vector3<T>& v2)
        {
            return vector3<T>(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
        }

        template <typename T>
        MAG_API inline T max(const T v1, const T v2)
        {
            return v1 > v2 ? v1 : v2;
        }

        // Returns the maximum value for each pair
        template <typename T>
        MAG_API inline vector3<T> max(const vector3<T>& v1, const vector3<T>& v2)
        {
            return vector3<T>(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
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
