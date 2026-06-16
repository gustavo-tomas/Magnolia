#pragma once

#include <vector>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/types.hpp"

namespace mag
{
    namespace math
    {
        template <typename T>
        struct vector2;

        template <typename T>
        struct vector3;

        template <typename T>
        struct vector4;

        template <typename T>
        struct quaternion;

        template <typename T>
        struct matrix3x3;

        template <typename T>
        struct matrix4x4;

        // Shortcuts

        using vec2 = vector2<f32>;
        using vec3 = vector3<f32>;
        using vec4 = vector4<f32>;

        using ivec2 = vector2<i32>;
        using ivec3 = vector3<i32>;
        using ivec4 = vector4<i32>;

        using uvec2 = vector2<u32>;
        using uvec3 = vector3<u32>;
        using uvec4 = vector4<u32>;

        using quat = quaternion<f32>;

        using mat3 = matrix3x3<f32>;
        using mat4 = matrix4x4<f32>;

        // Vector2

        template <typename T>
        MAG_API struct vector2
        {
                vector2() = default;
                vector2(const T scalar) : x(scalar), y(scalar) {}

                template <typename X, typename Y>
                vector2(const X x, const Y y) : x(static_cast<T>(x)), y(static_cast<T>(y))
                {
                }

                template <typename U>
                constexpr vector2(const vector2<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
                {
                }

                template <typename U>
                constexpr vector2(const vector3<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
                {
                }

                union
                {
                        T x, r, s;
                };
                union
                {
                        T y, g, t;
                };

                T& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 2, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return x;
                        case 1:
                            return y;
                    };
                }

                const T& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 2, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return x;
                        case 1:
                            return y;
                    };
                }
        };

        template <typename T>
        MAG_API constexpr vector2<T> operator/(const vector2<T>& v, const f32 s)
        {
            return vector2<T>(v.x / s, v.y / s);
        }

        template <typename T>
        MAG_API constexpr vector2<T> operator+(const vector2<T>& v1, const vector2<T>& v2)
        {
            return vector2<T>(v1.x + v2.x, v1.y + v2.y);
        }

        template <typename T>
        MAG_API constexpr vector2<T> operator-(const vector2<T>& v1, const vector2<T>& v2)
        {
            return vector2<T>(v1.x - v2.x, v1.y - v2.y);
        }

        // Vector 3

        template <typename T>
        MAG_API struct vector3
        {
                vector3() = default;
                vector3(const T scalar) : x(scalar), y(scalar), z(scalar) {}
                vector3(const T x, const T y, const T z) : x(x), y(y), z(z) {}
                vector3(const vector2<T>& v, const T z) : x(v.x), y(v.y), z(z) {}
                vector3(const vector4<T>& v) : x(v.x), y(v.y), z(v.z) {}

                union
                {
                        T x, r, s;
                };
                union
                {
                        T y, g, t;
                };
                union
                {
                        T z, b, p;
                };

                T& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 3, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return x;
                        case 1:
                            return y;
                        case 2:
                            return z;
                    };
                }

                const T& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 3, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return x;
                        case 1:
                            return y;
                        case 2:
                            return z;
                    };
                }

                template <typename U>
                vector3<T>& operator-=(const vector3<U>& v)
                {
                    x -= static_cast<T>(v.x);
                    y -= static_cast<T>(v.y);
                    z -= static_cast<T>(v.z);

                    return *this;
                }

                template <typename U>
                vector3<T>& operator+=(const vector3<U>& v)
                {
                    x += static_cast<T>(v.x);
                    y += static_cast<T>(v.y);
                    z += static_cast<T>(v.z);

                    return *this;
                }

                template <typename U>
                vector3<T>& operator*=(const vector3<U>& v)
                {
                    MAG_ASSERT(false, "@TODO");

                    return *this;
                }
        };

        template <typename T>
        MAG_API constexpr vector3<T> operator-(const vector3<T>& v)
        {
            return vector3<T>(-v.x, -v.y, -v.z);
        }

        template <typename T>
        MAG_API constexpr vector3<T> operator*(const vector3<T>& v, const f32 s)
        {
            return vector3<T>(v.x * s, v.y * s, v.z * s);
        }

        template <typename T>
        MAG_API constexpr vector3<T> operator+(const vector3<T>& v1, const vector3<T>& v2)
        {
            MAG_ASSERT(false, "@TODO");
        }

        template <typename T>
        MAG_API constexpr vector3<T> operator*(const vector3<T>& v1, const vector3<T>& v2)
        {
            MAG_ASSERT(false, "@TODO");
        }

        // Vector 4

        template <typename T>
        MAG_API struct vector4
        {
                vector4() = default;
                vector4(const T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
                vector4(const vector3<T>& v, const T w) : x(v.x), y(v.y), z(v.z), w(w) {}

                template <typename X, typename Y, typename Z, typename W>
                vector4(const X x, const Y y, const Z z, const W w)
                    : x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z)), w(static_cast<T>(w))
                {
                }

                union
                {
                        T x, r, s;
                };
                union
                {
                        T y, g, t;
                };
                union
                {
                        T z, b, p;
                };
                union
                {
                        T w, a, q;
                };

                T& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return x;
                        case 1:
                            return y;
                        case 2:
                            return z;
                        case 3:
                            return w;
                    };
                }

                const T& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return x;
                        case 1:
                            return y;
                        case 2:
                            return z;
                        case 3:
                            return w;
                    };
                }
        };

        template <typename T>
        MAG_API constexpr vector4<T> operator+(const vector4<T>& v1, const vector4<T>& v2)
        {
            return vector4<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator-(const vector4<T>& v1, const vector4<T>& v2)
        {
            return vector4<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
        }

        // Quaternion

        template <typename T>
        MAG_API struct quaternion
        {
                quaternion() = default;
                quaternion(const T w, const T x, const T y, const T z) : w(w), x(x), y(y), z(z) {}
                quaternion(const vector3<T>& v) { MAG_ASSERT(false, "@TODO"); }

                T w, x, y, z;

                T& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return w;
                        case 1:
                            return x;
                        case 2:
                            return y;
                        case 3:
                            return z;
                    };
                }

                const T& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);

                    switch (i)
                    {
                        default:
                        case 0:
                            return w;
                        case 1:
                            return x;
                        case 2:
                            return y;
                        case 3:
                            return z;
                    };
                }
        };

        // Mat 3x3

        template <typename T>
        MAG_API struct matrix3x3
        {
                matrix3x3() = default;
                matrix3x3(const T s) : data(vector3<T>(s, 0, 0), vector3<T>(0, s, 0), vector3<T>(0, 0, s)) {}
                matrix3x3(const vector3<T>& v0, const vector3<T>& v1, const vector3<T>& v2) : data{v0, v1, v2} {}

                vector3<T>& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 3, "Out of bounds index: {0}", i);
                    return data[i];
                }

                const vector3<T>& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 3, "Out of bounds index: {0}", i);
                    return data[i];
                }

            private:
                std::array<vector3<T>, 3> data;
        };

        template <typename T>
        MAG_API constexpr vector3<T> operator*(const matrix3x3<T>& m, const vector3<T>& v)
        {
            MAG_ASSERT(false, "@TODO");
        }

        // Mat 4x4

        template <typename T>
        MAG_API struct matrix4x4
        {
                matrix4x4() = default;
                matrix4x4(const T s)
                    : data{vector4<T>(s, 0, 0, 0), vector4<T>(0, s, 0, 0), vector4<T>(0, 0, s, 0),
                           vector4<T>(0, 0, 0, s)}
                {
                }

                vector4<T>& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);
                    return data[i];
                }

                const vector4<T>& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);
                    return data[i];
                }

            private:
                std::array<vector4<T>, 4> data;
        };

        template <typename T>
        MAG_API constexpr matrix4x4<T> operator*(const matrix4x4<T>& m1, const matrix4x4<T>& m2)
        {
            MAG_ASSERT(false, "@TODO");
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator*(const matrix4x4<T>& m, const vector4<T>& v)
        {
            MAG_ASSERT(false, "@TODO");
        }

        // Represents a simple triangle
        struct MAG_API Triangle
        {
                math::vec3 v0;
                math::vec3 v1;
                math::vec3 v2;
        };

        // A structure that represents a line that goes from start to end.
        struct MAG_API Line
        {
                vec3 start;
                vec3 end;
                vec3 color;
        };

        // Sequence of lines. Starts, ends and colors size must match.
        struct MAG_API LineList
        {
                std::vector<Line> lines;

                void append(const Line& line);
        };

        // Axis Aligned Bounding Box
        struct MAG_API BoundingBox
        {
                vec3 min;
                vec3 max;

                // Helper method to calculate bounding box after a transformation.
                BoundingBox get_transformed_bounding_box(const mat4& transform) const;

                // Helper method to get the list of edges.
                LineList get_line_list(const mat4& transform) const;
        };
    };  // namespace math
};  // namespace mag
