#pragma once

// Wrapper for glm

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>

#include "core/types.hpp"

namespace mag
{
    typedef glm::vec2 vec2;
    typedef glm::vec3 vec3;
    typedef glm::vec4 vec4;

    typedef glm::uvec2 uvec2;
    typedef glm::uvec3 uvec3;
    typedef glm::uvec4 uvec4;

    typedef glm::ivec2 ivec2;
    typedef glm::ivec3 ivec3;
    typedef glm::ivec4 ivec4;

    typedef glm::mat3 mat3;
    typedef glm::mat4 mat4;

    const f32 Half_Pi = glm::half_pi<f32>();
    const f32 Pi = glm::pi<f32>();
    const f32 Two_Pi = glm::two_pi<f32>();

    template <typename T>
    inline str to_str(const T& num)
    {
        return glm::to_string(num);
    }
};  // namespace mag
