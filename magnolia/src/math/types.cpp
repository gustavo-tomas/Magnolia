#include "math/types.hpp"

namespace mag::math
{
    b8 decompose_simple(const mat4& model_matrix, vec3& scale, vec3& rotation, vec3& translation)
    {
        quat orientation;
        vec3 skew;
        vec4 perspective;

        const b8 result = glm::decompose(model_matrix, scale, orientation, translation, skew, perspective);
        rotation = degrees(glm::eulerAngles(orientation));

        return result;
    }

    void LineList::append(const LineList& lines)
    {
        starts.insert(starts.begin(), lines.starts.begin(), lines.starts.end());
        ends.insert(ends.begin(), lines.ends.begin(), lines.ends.end());
        colors.insert(colors.begin(), lines.colors.begin(), lines.colors.end());
    }

    // @TODO: DRY helper methods
    BoundingBox BoundingBox::get_transformed_bounding_box(const mat4& transform) const
    {
        BoundingBox transformed_aabb;

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

            lines.starts.push_back(corners[start]);
            lines.ends.push_back(corners[end]);
            lines.colors.push_back(color);
        }

        return lines;
    };
};  // namespace mag::math