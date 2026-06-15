#include "magnolia/math/types.hpp"

#include <random>

#include "magnolia/core/logger.hpp"

namespace mag
{
    namespace math
    {
        /////////////
        // @TODO: separate .hpp and .cpp when finished

        template <typename T>
        struct vector2
        {
                vector2() = default;
                vector2(const T scalar) : x(scalar), y(scalar) {}
                vector2(const T x, const T y) : x(x), y(y) {}

                union
                {
                        T x, r, s;
                };
                union
                {
                        T y, g, t;
                };

                T& operator[](const u64 i)
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

                const T& operator[](const u64 i) const
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
        constexpr vector2<T> operator+(const vector2<T>& v1, const vector2<T>& v2)
        {
            return vector2<T>(v1.x + v2.x, v1.y + v2.y);
        }

        // Shortcuts

        using vec2 = vector2<f32>;
        using ivec2 = vector2<i32>;
        using uvec2 = vector2<u32>;

        template <typename T>
        constexpr str to_string(const vector2<T>& v, const u8 precision = 3)
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

        // @TODO: temp
        void test()
        {
            // Vec2
            {
                vec2 v = {};
                MAG_ASSERT(v.x == v[0], "Component mismatch: {0}", v.x);
                MAG_ASSERT(v.y == v[1], "Component mismatch: {0}", v.y);
                // v[2]; // out of bounds

                {
                    vec2 v0(1.2, 2.3);
                    vec2 v1(3.2, 4.3);
                    vec2 v3 = v0 + v1;

                    LOG_INFO("RES: {0}", to_string(v3));
                }

                {
                    ivec2 v0(1, -2);
                    ivec2 v1(-3, 4);
                    ivec2 v3 = v0 + v1;

                    LOG_INFO("RES: {0}", to_string(v3));
                }

                {
                    uvec2 v0(1, 2);
                    uvec2 v1(3, 4);
                    uvec2 v3 = v0 + v1;

                    LOG_INFO("RES: {0}", to_string(v3));
                }
            }
        }

        /////////////

        struct State
        {
                std::mt19937 random_engine;
                std::uniform_int_distribution<std::mt19937::result_type> distribution;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            test();

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

        b8 decompose_simple(const mat4& model_matrix, vec3& scale, quat& rotation, vec3& translation)
        {
            quat orientation;
            vec3 skew;
            vec4 perspective;

            const b8 result = glm::decompose(model_matrix, scale, orientation, translation, skew, perspective);
            rotation = orientation;

            return result;
        }

        mat4 calculate_rotation_mat(const vec3& rotation)
        {
            const quat pitch_rotation = angleAxis(rotation.x, vec3(1, 0, 0));
            const quat yaw_rotation = angleAxis(rotation.y, vec3(0, 1, 0));
            const quat roll_rotation = angleAxis(rotation.z, vec3(0, 0, 1));

            const mat4 rotation_mat = toMat4(roll_rotation) * toMat4(yaw_rotation) * toMat4(pitch_rotation);

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

        void LineList::append(const Line& line) { lines.push_back(line); }

        BoundingBox BoundingBox::get_transformed_bounding_box(const mat4& transform) const
        {
            BoundingBox transformed_aabb = {};

            // Remove translation influence
            mat4 model_without_transform = transform;
            model_without_transform[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

            std::vector<vec3> vertices = {this->min,
                                          vec3(this->min.x, this->min.y, this->max.z),
                                          vec3(this->min.x, this->max.y, this->min.z),
                                          vec3(this->min.x, this->max.y, this->max.z),
                                          vec3(this->max.x, this->min.y, this->min.z),
                                          vec3(this->max.x, this->min.y, this->max.z),
                                          vec3(this->max.x, this->max.y, this->min.z),
                                          this->max};

            // Transform all vertices
            for (vec3& vertex : vertices)
            {
                vertex = model_without_transform * vec4(vertex, 1.0f);
            }

            // Recalculate min and max
            transformed_aabb.min = vertices[0];
            transformed_aabb.max = vertices[0];

            for (const vec3& vertex : vertices)
            {
                transformed_aabb.min = math::min(transformed_aabb.min, vertex);
                transformed_aabb.max = math::max(transformed_aabb.max, vertex);
            }

            // Re-apply translation
            transformed_aabb.min = translate(vec3(transform[3])) * vec4(transformed_aabb.min, 1.0f);
            transformed_aabb.max = translate(vec3(transform[3])) * vec4(transformed_aabb.max, 1.0f);

            return transformed_aabb;
        }

        LineList BoundingBox::get_line_list(const mat4& transform) const
        {
            const BoundingBox transformed_aabb = get_transformed_bounding_box(transform);

            const vec3& min_p = transformed_aabb.min;
            const vec3& max_p = transformed_aabb.max;

            // Generate the box corners
            std::vector<vec3> corners(8);

            corners[0] = min_p;
            corners[1] = vec3(min_p.x, min_p.y, max_p.z);
            corners[2] = vec3(min_p.x, max_p.y, min_p.z);
            corners[3] = vec3(min_p.x, max_p.y, max_p.z);
            corners[4] = vec3(max_p.x, min_p.y, min_p.z);
            corners[5] = vec3(max_p.x, min_p.y, max_p.z);
            corners[6] = vec3(max_p.x, max_p.y, min_p.z);
            corners[7] = max_p;

            // Generate the box edges
            std::vector<std::pair<u32, u32>> edges;

            edges = {
                {0, 1},
                {1, 3},
                {3, 2},
                {2, 0}, // Bottom face
                {4, 5},
                {5, 7},
                {7, 6},
                {6, 4}, // Top face
                {0, 4},
                {1, 5},
                {2, 6},
                {3, 7}, // Vertical edges
                                                 // {0, 7} // Diagonal
            };

            // Orange color
            const vec3 color = vec3(0.99, 0.68, 0.01);

            LineList lines;

            for (const auto& edge : edges)
            {
                const auto& start = edge.first;
                const auto& end = edge.second;

                Line line = {};

                line.start = corners[start];
                line.end = corners[end];
                line.color = color;

                lines.append(line);
            }

            return lines;
        };
    };  // namespace math
};  // namespace mag
