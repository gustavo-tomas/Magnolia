#pragma once

#include "core/types.hpp"
#include "glm/fwd.hpp"

// Math definitions
namespace mag::math
{
    // Wrapper for glm
    using namespace glm;

    // Simpler version of glm::decompose
    b8 decompose_simple(const mat4& model_matrix, vec3& scale, vec3& rotation, vec3& translation);

    // Calculate a rotation mat from XYZ rotation (degrees)
    mat4 calculate_rotation_mat(const vec3& rotation);

    // Sequence of lines. Starts, ends and colors size must match.
    struct LineList;

    // Axis Aligned Bounding Box
    struct BoundingBox;
};  // namespace mag::math
