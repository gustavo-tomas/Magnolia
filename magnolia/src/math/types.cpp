#include "magnolia/math/types.hpp"

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

        void LineList::append(const Line& line) { lines.push_back(line); }

        // @TODO: DRY helper methods
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
            for (auto& vertex : vertices)
            {
                vertex = model_without_transform * vec4(vertex, 1.0f);
            }

            // Recalculate min and max
            transformed_aabb.min = vertices[0];
            transformed_aabb.max = vertices[0];

            for (const auto& vertex : vertices)
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
                {0, 1}, {1, 3}, {3, 2}, {2, 0},  // Bottom face
                {4, 5}, {5, 7}, {7, 6}, {6, 4},  // Top face
                {0, 4}, {1, 5}, {2, 6}, {3, 7},  // Vertical edges
                // {0, 7}                           // Diagonal
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
