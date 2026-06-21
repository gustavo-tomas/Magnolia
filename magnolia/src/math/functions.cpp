#include "magnolia/math/functions.hpp"

#include <random>

namespace mag
{
    namespace math
    {
        struct State
        {
                std::mt19937 random_engine = std::mt19937(std::random_device()());
                std::uniform_int_distribution<std::mt19937::result_type> distribution;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            return state != nullptr;
        }

        void shutdown() { delete state; }

        f32 radians(const f32 angle_deg)
        {
            const f32 pi_over_180 = 0.01745329251994329576923690768489F;
            return angle_deg * pi_over_180;
        }

        f32 dot(const vec3& v1, const vec3& v2)
        {
            const vec3 prod = v1 * v2;
            const f32 res = prod.x + prod.y + prod.z;

            return res;
        }

        f32 dot(const vec4& v1, const vec4& v2)
        {
            const f32 res = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);

            return res;
        }

        f32 dot(const quat& q1, const quat& q2)
        {
            const vec4 prod(q1.w * q2.w, q1.x * q2.x, q1.y * q2.y, q1.z * q2.z);
            const f32 res = (prod.x + prod.y) + (prod.z + prod.w);

            return res;
        }

        f32 length(const vec3& v)
        {
            const f32 res = std::sqrt(dot(v, v));

            return res;
        }

        f32 length(const quat& q)
        {
            const f32 res = std::sqrt(dot(q, q));

            return res;
        }

        vec3 normalize(const vec3& v)
        {
            const f32 d = length(v);

            // Return zero if length is invalid
            if (d <= 0.0F)
            {
                return {0.0F, 0.0F, 0.0F};
            }

            const f32 inv_d = 1.0F / d;
            const vec3 res = {v.x * inv_d, v.y * inv_d, v.z * inv_d};

            return res;
        }

        quat normalize(const quat& q)
        {
            const f32 d = length(q);

            // Return identity if length is invalid
            if (d <= 0.0F)
            {
                return {1.0F, 0.0F, 0.0F, 0.0F};
            }

            const f32 inv_d = 1.0F / d;
            const quat res = {q.w * inv_d, q.x * inv_d, q.y * inv_d, q.z * inv_d};

            return res;
        }

        mat4 translate(const mat4& m, const vec3& position)
        {
            mat4 res = m;
            res[3] += vec4(position, 0.0F);

            return res;
        }

        mat4 translate(const vec3& position)
        {
            mat4 res(1.0F);
            res[3] += vec4(position, 0.0F);

            return res;
        }

        mat4 scale(const mat4& m, const vec3& v)
        {
            mat4 res(0.0F);
            res[0] = m[0] * v[0];
            res[1] = m[1] * v[1];
            res[2] = m[2] * v[2];
            res[3] = m[3];

            return res;
        }

        mat4 inverse(const mat4& m)
        {
            const f32 coef00 = (m[2][2] * m[3][3]) - (m[3][2] * m[2][3]);
            const f32 coef02 = (m[1][2] * m[3][3]) - (m[3][2] * m[1][3]);
            const f32 coef03 = (m[1][2] * m[2][3]) - (m[2][2] * m[1][3]);

            const f32 coef04 = (m[2][1] * m[3][3]) - (m[3][1] * m[2][3]);
            const f32 coef06 = (m[1][1] * m[3][3]) - (m[3][1] * m[1][3]);
            const f32 coef07 = (m[1][1] * m[2][3]) - (m[2][1] * m[1][3]);

            const f32 coef08 = (m[2][1] * m[3][2]) - (m[3][1] * m[2][2]);
            const f32 coef10 = (m[1][1] * m[3][2]) - (m[3][1] * m[1][2]);
            const f32 coef11 = (m[1][1] * m[2][2]) - (m[2][1] * m[1][2]);

            const f32 coef12 = (m[2][0] * m[3][3]) - (m[3][0] * m[2][3]);
            const f32 coef14 = (m[1][0] * m[3][3]) - (m[3][0] * m[1][3]);
            const f32 coef15 = (m[1][0] * m[2][3]) - (m[2][0] * m[1][3]);

            const f32 coef16 = (m[2][0] * m[3][2]) - (m[3][0] * m[2][2]);
            const f32 coef18 = (m[1][0] * m[3][2]) - (m[3][0] * m[1][2]);
            const f32 coef19 = (m[1][0] * m[2][2]) - (m[2][0] * m[1][2]);

            const f32 coef20 = (m[2][0] * m[3][1]) - (m[3][0] * m[2][1]);
            const f32 coef22 = (m[1][0] * m[3][1]) - (m[3][0] * m[1][1]);
            const f32 coef23 = (m[1][0] * m[2][1]) - (m[2][0] * m[1][1]);

            const vec4 fac0(coef00, coef00, coef02, coef03);
            const vec4 fac1(coef04, coef04, coef06, coef07);
            const vec4 fac2(coef08, coef08, coef10, coef11);
            const vec4 fac3(coef12, coef12, coef14, coef15);
            const vec4 fac4(coef16, coef16, coef18, coef19);
            const vec4 fac5(coef20, coef20, coef22, coef23);

            const vec4 vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
            const vec4 vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
            const vec4 vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
            const vec4 vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

            const vec4 inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
            const vec4 inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
            const vec4 inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
            const vec4 inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

            const vec4 signa(+1, -1, +1, -1);
            const vec4 signb(-1, +1, -1, +1);
            mat4 res(inv0 * signa, inv1 * signb, inv2 * signa, inv3 * signb);

            const vec4 row0(res[0][0], res[1][0], res[2][0], res[3][0]);

            const vec4 dot0(m[0] * row0);
            const f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

            const f32 one_over_d = 1.0F / dot1;

            res *= one_over_d;

            return res;
        }

        mat4 transpose(const mat4& m)
        {
            mat4 res(0.0F);

            for (u32 i = 0; i < 4; i++)
            {
                for (u32 j = 0; j < 4; j++)
                {
                    res[i][j] = m[j][i];
                }
            }

            return res;
        }

        vec3 cross(const vec3& v1, const vec3& v2)
        {
            const vec3 res = {(v1.y * v2.z) - (v2.y * v1.z), (v1.z * v2.x) - (v2.z * v1.x),
                              (v1.x * v2.y) - (v2.x * v1.y)};

            return res;
        }

        quat to_quat(const mat4& m)
        {
            // glm people are smarter than me
            const f32 four_x_squared_minus1 = m[0][0] - m[1][1] - m[2][2];
            const f32 four_y_squared_minus1 = m[1][1] - m[0][0] - m[2][2];
            const f32 four_z_squared_minus1 = m[2][2] - m[0][0] - m[1][1];
            const f32 four_w_squared_minus1 = m[0][0] + m[1][1] + m[2][2];

            u32 biggest_index = 0;
            f32 four_biggest_squared_minus1 = four_w_squared_minus1;
            if (four_x_squared_minus1 > four_biggest_squared_minus1)
            {
                four_biggest_squared_minus1 = four_x_squared_minus1;
                biggest_index = 1;
            }
            if (four_y_squared_minus1 > four_biggest_squared_minus1)
            {
                four_biggest_squared_minus1 = four_y_squared_minus1;
                biggest_index = 2;
            }
            if (four_z_squared_minus1 > four_biggest_squared_minus1)
            {
                four_biggest_squared_minus1 = four_z_squared_minus1;
                biggest_index = 3;
            }

            const f32 biggest_val = std::sqrt(four_biggest_squared_minus1 + 1.0F) * 0.5F;
            const f32 mult = 0.25F / biggest_val;

            quat res;

            switch (biggest_index)
            {
                case 0:
                    res = {biggest_val, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult,
                           (m[0][1] - m[1][0]) * mult};
                    break;

                case 1:
                    res = {(m[1][2] - m[2][1]) * mult, biggest_val, (m[0][1] + m[1][0]) * mult,
                           (m[2][0] + m[0][2]) * mult};
                    break;

                case 2:
                    res = {(m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggest_val,
                           (m[1][2] + m[2][1]) * mult};
                    break;

                case 3:
                    res = {(m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult,
                           biggest_val};
                    break;

                default:
                    MAG_ASSERT(false, "Failed to convert rotation matrix to quaternion");
                    res = {1, 0, 0, 0};
                    break;
            }

            return res;
        }

        mat4 to_mat4(const quat& q)
        {
            const f32 w = q.w;
            const f32 x = q.x;
            const f32 y = q.y;
            const f32 z = q.z;

            const f32 xx = x * x;
            const f32 yy = y * y;
            const f32 zz = z * z;

            const f32 xy = x * y;
            const f32 xz = x * z;
            const f32 yz = y * z;

            const f32 wx = w * x;
            const f32 wy = w * y;
            const f32 wz = w * z;

            const mat4 res = {
                {1.0F - (2.0F * (yy + zz)), 2.0F * (xy + wz),          2.0F * (xz - wy),          0.0F},
                {2.0F * (xy - wz),          1.0F - (2.0F * (xx + zz)), 2.0F * (yz + wx),          0.0F},
                {2.0F * (xz + wy),          2.0F * (yz - wx),          1.0F - (2.0F * (xx + yy)), 0.0F},
                {0.0F,                      0.0F,                      0.0F,                      1.0F},
            };

            return res;
        }

        mat4 perspective(const f32 fov, const f32 aspect, const f32 near, const f32 far)
        {
            const mat4 res = {
                {1.0F / (aspect * std::tan(fov / 2.0F)), 0.0F,                        0.0F,                                0.0F },
                {0.0F,                                   1.0F / std::tan(fov / 2.0F), 0.0F,                                0.0F },
                {0.0F,                                   0.0F,                        -(far + near) / (far - near),        -1.0F},
                {0.0F,                                   0.0F,                        (-2.0F * far * near) / (far - near), 0.0F },
            };

            return res;
        }

        mat4 ortho(const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 near, const f32 far)
        {
            const mat4 res = {
                {2.0F / (right - left),            0.0F,                             0.0F,                        0.0F},
                {0.0F,                             2.0F / (top - bottom),            0.0F,                        0.0F},
                {0.0F,                             0.0F,                             2.0F / (near - far),         0.0F},
                {-(right + left) / (right - left), -(top + bottom) / (top - bottom), (near + far) / (near - far), 1.0F},
            };

            return res;
        }

        f32 random(const f32 begin, const f32 end)
        {
            f32 b = begin;
            f32 e = end;

            // Swap values
            if (begin > end)
            {
                std::swap(b, e);
            }

            // Number is a value between 0 and 1
            const f32 number = static_cast<f32>(state->distribution(state->random_engine)) /
                               static_cast<f32>(std::numeric_limits<uint_fast32_t>::max());

            return b + ((e - b) * number);
        }

        i32 random(const i32 begin, const i32 end)
        {
            return static_cast<i32>(random(static_cast<f32>(begin), static_cast<f32>(end)));
        }

        vec3 get_right_dir(const f32 yaw)
        {
            vec3 right(0.0F);
            right.x = cos(yaw);
            right.y = 0;
            right.z = -sin(yaw);

            return right;
        }

        vec3 get_forward_dir(const f32 pitch, const f32 yaw)
        {
            vec3 forward(0.0F);
            forward.x = cos(-pitch) * sin(yaw);
            forward.y = sin(-pitch);
            forward.z = cos(-pitch) * cos(yaw);

            return forward;
        }

        vec3 get_up_dir(const f32 pitch, const f32 yaw)
        {
            const vec3 forward = get_forward_dir(pitch, yaw);
            const vec3 right = get_right_dir(yaw);

            return cross(forward, right);
        }
    };  // namespace math
};  // namespace mag
