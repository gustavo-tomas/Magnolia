#pragma once

// Wrapper for glm

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "core/types.hpp"

namespace mag
{
    namespace math
    {
        using namespace glm;

        const f32 Half_Pi = glm::half_pi<f32>();
        const f32 Pi = glm::pi<f32>();
        const f32 Two_Pi = glm::two_pi<f32>();
    };  // namespace math
};      // namespace mag
