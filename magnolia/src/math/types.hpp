#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

// #include <glm/fwd.hpp> // @TODO: use this header

#include "core/types.hpp"

// Math definitions
namespace mag::math
{
    // Wrapper for glm
    using namespace glm;

    // Simpler version of glm::decompose
    b8 decompose_simple(const mat4& model_matrix, vec3& scale, vec3& rotation, vec3& translation);

    // Sequence of lines. Starts, ends and colors size must match.
    struct LineList
    {
            std::vector<vec3> starts, ends, colors;

            void append(const LineList& lines);
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
};  // namespace mag::math
