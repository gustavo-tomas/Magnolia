#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    using namespace mag::math;

    // Taken from: https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
    class Frustum
    {
        public:
            Frustum();
            Frustum(mat4 m);  // m = ProjectionMatrix * ViewMatrix
            Frustum(const Frustum& other);
            Frustum& operator=(const Frustum& other);
            ~Frustum();

            // https://iquilezles.org/articles/frustumcorrect/
            b8 is_aabb_visible(const BoundingBox& aabb) const;
            std::vector<vec3> get_points() const;

        private:
            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
