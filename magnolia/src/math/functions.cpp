#include "magnolia/math/functions.hpp"

#include <random>

namespace mag
{
    namespace math
    {
        struct State
        {
                std::mt19937 random_engine;
                std::uniform_int_distribution<std::mt19937::result_type> distribution;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            state->random_engine.seed(std::random_device()());

            return state != nullptr;
        }

        void shutdown() { delete state; }

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
            f32 number = static_cast<f32>(state->distribution(state->random_engine)) /
                         static_cast<f32>(std::numeric_limits<uint_fast32_t>::max());

            return b + ((e - b) * number);
        }

        i32 random(const i32 begin, const i32 end)
        {
            return static_cast<i32>(random(static_cast<f32>(begin), static_cast<f32>(end)));
        }

        mat4 calculate_rotation_mat(const vec3& rotation)
        {
            const quat pitch_rotation = angle_axis(rotation.x, vec3(1, 0, 0));
            const quat yaw_rotation = angle_axis(rotation.y, vec3(0, 1, 0));
            const quat roll_rotation = angle_axis(rotation.z, vec3(0, 0, 1));

            const mat4 rotation_mat = to_mat4(roll_rotation) * to_mat4(yaw_rotation) * to_mat4(pitch_rotation);

            return rotation_mat;
        }

        vec3 get_right_dir(const f32 yaw)
        {
            vec3 right(0.0f);
            right.x = cos(yaw);
            right.y = 0;
            right.z = -sin(yaw);

            return right;
        }

        vec3 get_forward_dir(const f32 pitch, const f32 yaw)
        {
            vec3 forward(0.0f);
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
