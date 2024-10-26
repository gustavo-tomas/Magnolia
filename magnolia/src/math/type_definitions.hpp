#pragma once

#include <vector>

#include "glm/vec3.hpp"
#include "math/types.hpp"

namespace mag::math
{
    struct LineList
    {
            std::vector<vec3> starts, ends, colors;

            void append(const LineList& lines);
    };

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
