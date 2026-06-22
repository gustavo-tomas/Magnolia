#pragma once

#include <cmath>
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
        struct MAG_API vector2
        {
                vector2() : x(0), y(0) {}
                explicit vector2(const T scalar) : x(scalar), y(scalar) {}

                template <typename X, typename Y>
                vector2(const X x, const Y y) : x(static_cast<T>(x)), y(static_cast<T>(y))
                {
                }

                template <typename U>
                explicit vector2(const vector2<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
                {
                }

                template <typename U>
                explicit vector2(const vector3<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
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

                constexpr T& operator[](const u8 i)
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

                constexpr const T& operator[](const u8 i) const
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

                template <typename U>
                constexpr vector2<T>& operator+=(const vector2<U>& v)
                {
                    x += static_cast<T>(v.x);
                    y += static_cast<T>(v.y);

                    return *this;
                }

                template <typename U>
                constexpr vector2<T>& operator-=(const vector2<U>& v)
                {
                    x -= static_cast<T>(v.x);
                    y -= static_cast<T>(v.y);

                    return *this;
                }

                template <typename U>
                constexpr vector2<T>& operator*=(const vector2<U>& v)
                {
                    x *= static_cast<T>(v.x);
                    y *= static_cast<T>(v.y);

                    return *this;
                }

                template <typename U>
                constexpr vector2<T>& operator/=(const vector2<U>& v)
                {
                    x /= static_cast<T>(v.x);
                    y /= static_cast<T>(v.y);

                    return *this;
                }

                template <typename U>
                constexpr vector2<T>& operator/=(const U scalar)
                {
                    x /= static_cast<T>(scalar);
                    y /= static_cast<T>(scalar);

                    return *this;
                }
        };

        template <typename T>
        MAG_API constexpr vector2<T> operator+(const vector2<T>& v1, const vector2<T>& v2)
        {
            return vector2<T>(v1) += v2;
        }

        template <typename T>
        MAG_API constexpr vector2<T> operator-(const vector2<T>& v1, const vector2<T>& v2)
        {
            return vector2<T>(v1) -= v2;
        }

        template <typename T>
        MAG_API constexpr vector2<T> operator/(const vector2<T>& v, const T s)
        {
            return vector2<T>(v) /= s;
        }

        // Vector 3

        template <typename T>
        struct MAG_API vector3
        {
                vector3() : x(0), y(0), z(0) {}
                explicit vector3(const T scalar) : x(scalar), y(scalar), z(scalar) {}
                vector3(const T x, const T y, const T z) : x(x), y(y), z(z) {}
                vector3(const vector2<T>& v, const T z) : x(v.x), y(v.y), z(z) {}
                explicit vector3(const vector4<T>& v) : x(v.x), y(v.y), z(v.z) {}

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

                constexpr T& operator[](const u8 i)
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

                constexpr const T& operator[](const u8 i) const
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
                constexpr vector3<T>& operator+=(const vector3<U>& v)
                {
                    x += static_cast<T>(v.x);
                    y += static_cast<T>(v.y);
                    z += static_cast<T>(v.z);

                    return *this;
                }

                template <typename U>
                constexpr vector3<T>& operator-=(const vector3<U>& v)
                {
                    x -= static_cast<T>(v.x);
                    y -= static_cast<T>(v.y);
                    z -= static_cast<T>(v.z);

                    return *this;
                }

                template <typename U>
                constexpr vector3<T>& operator*=(const U scalar)
                {
                    x *= static_cast<T>(scalar);
                    y *= static_cast<T>(scalar);
                    z *= static_cast<T>(scalar);

                    return *this;
                }

                template <typename U>
                constexpr vector3<T>& operator*=(const vector3<U>& v)
                {
                    x *= static_cast<T>(v.x);
                    y *= static_cast<T>(v.y);
                    z *= static_cast<T>(v.z);

                    return *this;
                }
        };

        template <typename T>
        MAG_API constexpr vector3<T> operator+(const vector3<T>& v1, const vector3<T>& v2)
        {
            return vector3<T>(v1) += v2;
        }

        template <typename T>
        MAG_API constexpr vector3<T> operator-(const vector3<T>& v)
        {
            return vector3<T>(0) -= v;
        }

        template <typename T>
        MAG_API constexpr vector3<T> operator*(const vector3<T>& v, const T s)
        {
            return vector3<T>(v) *= s;
        }

        template <typename T>
        MAG_API constexpr vector3<T> operator*(const vector3<T>& v1, const vector3<T>& v2)
        {
            return vector3<T>(v1) *= v2;
        }

        // Vector 4

        template <typename T>
        struct MAG_API vector4
        {
                vector4() : x(0), y(0), z(0), w(0) {}
                explicit vector4(const T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
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

                constexpr T& operator[](const u8 i)
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

                constexpr const T& operator[](const u8 i) const
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

                template <typename U>
                constexpr vector4<T>& operator+=(const vector4<U>& v)
                {
                    x += static_cast<T>(v.x);
                    y += static_cast<T>(v.y);
                    z += static_cast<T>(v.z);
                    w += static_cast<T>(v.w);

                    return *this;
                }

                template <typename U>
                constexpr vector4<T>& operator-=(const vector4<U>& v)
                {
                    x -= static_cast<T>(v.x);
                    y -= static_cast<T>(v.y);
                    z -= static_cast<T>(v.z);
                    w -= static_cast<T>(v.w);

                    return *this;
                }

                template <typename U>
                constexpr vector4<T>& operator*=(const U scalar)
                {
                    x *= static_cast<T>(scalar);
                    y *= static_cast<T>(scalar);
                    z *= static_cast<T>(scalar);
                    w *= static_cast<T>(scalar);

                    return *this;
                }

                template <typename U>
                constexpr vector4<T>& operator*=(const vector4<U>& v)
                {
                    x *= static_cast<T>(v.x);
                    y *= static_cast<T>(v.y);
                    z *= static_cast<T>(v.z);
                    w *= static_cast<T>(v.w);

                    return *this;
                }
        };

        template <typename T>
        MAG_API constexpr b8 operator==(const vector4<T>& v1, const vector4<T>& v2)
        {
            return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator+(const vector4<T>& v1, const vector4<T>& v2)
        {
            return vector4<T>(v1) += v2;
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator-(const vector4<T>& v1, const vector4<T>& v2)
        {
            return vector4<T>(v1) -= v2;
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator*(const vector4<T>& v, const T s)
        {
            return vector4<T>(v) *= s;
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator*(const vector4<T>& v1, const vector4<T>& v2)
        {
            return vector4<T>(v1) *= v2;
        }

        // Quaternion

        template <typename T>
        struct MAG_API quaternion
        {
                quaternion() : w(1), x(0), y(0), z(0) {}
                quaternion(const T w, const T x, const T y, const T z) : w(w), x(x), y(y), z(z) {}
                explicit quaternion(const vector3<T>& euler_angle)
                {
                    const vec3 c = cos(euler_angle * static_cast<T>(0.5));
                    const vec3 s = sin(euler_angle * static_cast<T>(0.5));

                    w = (c.x * c.y * c.z) + (s.x * s.y * s.z);
                    x = (s.x * c.y * c.z) - (c.x * s.y * s.z);
                    y = (c.x * s.y * c.z) + (s.x * c.y * s.z);
                    z = (c.x * c.y * s.z) - (s.x * s.y * c.z);
                }

                T w, x, y, z;

                constexpr T& operator[](const u8 i)
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

                constexpr const T& operator[](const u8 i) const
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

                constexpr quaternion<T>& operator+=(const quaternion<T>& q)
                {
                    w += q.w;
                    x += q.x;
                    y += q.y;
                    z += q.z;

                    return *this;
                }

                constexpr quaternion<T>& operator-=(const quaternion<T>& q)
                {
                    w -= q.w;
                    x -= q.x;
                    y -= q.y;
                    z -= q.z;

                    return *this;
                }

                constexpr quaternion<T>& operator*=(const T s)
                {
                    w *= s;
                    x *= s;
                    y *= s;
                    z *= s;

                    return *this;
                }

                template <typename U>
                constexpr quaternion<T>& operator*=(const quaternion<U>& r)
                {
                    const quaternion<T> p(*this);
                    const quaternion<T> q(r);

                    w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
                    x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
                    y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
                    z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;

                    return *this;
                }
        };

        template <typename T>
        MAG_API constexpr quaternion<T> operator+(const quaternion<T>& q1, const quaternion<T>& q2)
        {
            return quaternion<T>(q1) += q2;
        }

        template <typename T>
        MAG_API constexpr quaternion<T> operator-(const quaternion<T>& q1, const quaternion<T>& q2)

        {
            return quaternion<T>(q1) -= q2;
        }

        template <typename T>
        MAG_API constexpr quaternion<T> operator*(const quaternion<T>& q1, const T s)
        {
            return quaternion<T>(q1) *= s;
        }

        template <typename T>
        MAG_API constexpr quaternion<T> operator*(const quaternion<T>& q1, const quaternion<T>& q2)
        {
            return quaternion<T>(q1) *= q2;
        }

        // Mat 3x3

        template <typename T>
        struct MAG_API matrix3x3
        {
                matrix3x3() = default;
                explicit matrix3x3(const T s) : columns(vector3<T>(s, 0, 0), vector3<T>(0, s, 0), vector3<T>(0, 0, s))
                {
                }
                matrix3x3(const vector3<T>& v0, const vector3<T>& v1, const vector3<T>& v2) : columns{v0, v1, v2} {}

                constexpr vector3<T>& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 3, "Out of bounds index: {0}", i);
                    return columns[i];
                }

                constexpr const vector3<T>& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 3, "Out of bounds index: {0}", i);
                    return columns[i];
                }

                template <typename U>
                constexpr vector3<T> operator*=(const vector3<U>& v)
                {
                    vector3<T> res(0);

                    for (u32 j = 0; j < 3; j++)
                    {
                        for (u32 i = 0; i < 3; i++)
                        {
                            res[i] += (*this)[j][i] * static_cast<T>(v[j]);
                        }
                    }

                    return res;
                }

            private:
                std::array<vector3<T>, 3> columns;
        };

        template <typename T>
        MAG_API constexpr vector3<T> operator*(const matrix3x3<T>& m, const vector3<T>& v)
        {
            return matrix3x3<T>(m) *= v;
        }

        // Mat 4x4

        template <typename T>
        struct MAG_API matrix4x4
        {
                matrix4x4() = default;
                explicit matrix4x4(const T s)
                    : columns{vector4<T>(s, 0, 0, 0), vector4<T>(0, s, 0, 0), vector4<T>(0, 0, s, 0),
                              vector4<T>(0, 0, 0, s)}
                {
                }

                matrix4x4(const vector4<T>& v0, const vector4<T>& v1, const vector4<T>& v2, const vector4<T>& v3)
                    : columns{v0, v1, v2, v3}
                {
                }

                constexpr vector4<T>& operator[](const u8 i)
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);
                    return columns[i];
                }

                constexpr const vector4<T>& operator[](const u8 i) const
                {
                    MAG_ASSERT(i < 4, "Out of bounds index: {0}", i);
                    return columns[i];
                }

                template <typename U>
                constexpr matrix4x4<T>& operator*=(const U s)
                {
                    for (u32 j = 0; j < 4; j++)
                    {
                        for (u32 i = 0; i < 4; i++)
                        {
                            (*this)[j][i] *= static_cast<T>(s);
                        }
                    }

                    return *this;
                }

                template <typename U>
                constexpr vector4<T> operator*=(const vector4<U>& v)
                {
                    vector4<T> res(0);

                    for (u32 j = 0; j < 4; j++)
                    {
                        for (u32 i = 0; i < 4; i++)
                        {
                            res[i] += (*this)[j][i] * static_cast<T>(v[j]);
                        }
                    }

                    return res;
                }

                template <typename U>
                constexpr matrix4x4<T> operator*=(const matrix4x4<U>& m)
                {
                    matrix4x4<T> res(0);

                    for (u32 j = 0; j < 4; j++)
                    {
                        for (u32 k = 0; k < 4; k++)
                        {
                            for (u32 i = 0; i < 4; i++)
                            {
                                res[j][i] += (*this)[k][i] * static_cast<T>(m[j][k]);
                            }
                        }
                    }

                    return res;
                }

            private:
                std::array<vector4<T>, 4> columns;
        };

        template <typename T>
        MAG_API constexpr b8 operator==(const matrix4x4<T>& m1, const matrix4x4<T>& m2)
        {
            return m1[0] == m2[0] && m1[1] == m2[1] && m1[2] == m2[2] && m1[3] == m2[3];
        }

        template <typename T>
        MAG_API constexpr matrix4x4<T> operator*(const matrix4x4<T>& m, const T s)
        {
            return matrix4x4<T>(m) *= s;
        }

        template <typename T>
        MAG_API constexpr vector4<T> operator*(const matrix4x4<T>& m, const vector4<T>& v)
        {
            return matrix4x4<T>(m) *= v;
        }

        template <typename T>
        MAG_API constexpr matrix4x4<T> operator*(const matrix4x4<T>& m1, const matrix4x4<T>& m2)
        {
            return matrix4x4<T>(m1) *= m2;
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
