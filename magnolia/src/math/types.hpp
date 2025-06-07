#pragma once

#include <vector>

#include "core/types.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/mat4x4.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

// @TODO: ideally the math types should be in the mag namespace and the math functions in the math namespace

// Math definitions
namespace mag
{
    namespace math
    {
        // Wrapper for glm
        using namespace glm;

        // Simpler version of glm::decompose
        b8 decompose_simple(const mat4& model_matrix, vec3& scale, vec3& rotation, vec3& translation);

        // Calculate a rotation mat from XYZ rotation
        mat4 calculate_rotation_mat(const vec3& rotation);

        // A structure that represents a line that goes from start to end.
        struct Line
        {
                vec3 start;
                vec3 end;
                vec3 color;
        };

        // Sequence of lines. Starts, ends and colors size must match.
        struct LineList
        {
                std::vector<Line> lines;

                void append(const Line& line);
        };

        // Axis Aligned Bounding Box
        struct BoundingBox
        {
                vec3 min;
                vec3 max;

                // Helper method to calculate bounding box after a transformation.
                BoundingBox get_transformed_bounding_box(const mat4& transform) const;

                // Helper method to get the list of edges.
                LineList get_line_list(const mat4& transform) const;
        };
    };  // namespace math
};      // namespace mag
