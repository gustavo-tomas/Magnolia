#pragma once

#include <vector>

#include "glm/fwd.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"
#include "magnolia/core/types.hpp"

// Math definitions
namespace mag
{
    namespace math
    {
        // Wrapper for glm
        using namespace glm;

        b8 initialize();

        void shutdown();

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

        MAG_API f32 random(const f32 begin = 0.0f, const f32 end = 1.0f);

        MAG_API i32 random(const i32 begin = 0, const i32 end = 1);

        // Simpler version of glm::decompose
        MAG_API b8 decompose_simple(const mat4& model_matrix, vec3& scale, quat& rotation, vec3& translation);

        // Calculate a rotation mat from XYZ rotation
        MAG_API mat4 calculate_rotation_mat(const vec3& rotation);

        // Get direction from angles
        MAG_API vec3 get_right_dir(const f32 yaw);

        MAG_API vec3 get_forward_dir(const f32 pitch, const f32 yaw);

        MAG_API vec3 get_up_dir(const f32 pitch, const f32 yaw);
    };  // namespace math
};  // namespace mag
