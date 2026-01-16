#if defined(__cplusplus)
    #pragma once

    #include "magnolia/core/types.hpp"
    #include "magnolia/math/types.hpp"

using namespace mag;
using namespace mag::math;

#else

// See this: https://developer.nvidia.com/vulkan-shader-resource-binding
    #extension GL_EXT_nonuniform_qualifier : require

    #define f32 float
    #define u32 uint
    #define alignas(x)

// Constants
const float PI = 3.1415926535;

#endif

// Types shared by c++ and glsl

struct alignas(16) DebugTextData
{
        mat4 model;       // 64 bytes (16 x 4)
        vec4 color;       // 16 bytes ( 4 x 4)
        u32 texture_idx;  // 4 bytes
};
