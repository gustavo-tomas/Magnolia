#if defined(__cplusplus)
    #pragma once

    #include "core/types.hpp"
    #include "math/types.hpp"

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

// Including unbounded arrays
const u32 Max_Descriptor_Array_Size = 1000;

// Types shared by c++ and glsl

struct alignas(16) MeshData
{
        mat4 model;        // 64 bytes (16 x 4)
        u32 material_idx;  // 4 bytes
};

struct alignas(16) LightData
{
        vec3 color;     // 12 bytes (3 x 4)
        f32 intensity;  // 4 bytes  (1 x 4)
        vec3 position;  // 12 bytes (3 x 4)
};

struct alignas(16) SpriteData
{
        mat4 model;            // 64 bytes
        vec4 size_const_face;  // Size + Constant Size + Always Face Camera
        u32 texture_idx;
};

struct alignas(16) TextData
{
        mat4 model;  // 64 bytes (16 x 4)
        vec4 color;  // 16 bytes ( 4 x 4)
};

struct alignas(16) MaterialData
{
        vec4 albedo;            // 16 bytes
        f32 roughness;          // 4 bytes
        f32 metallic;           // 4 bytes
        u32 albedo_tex_idx;     // 4 bytes
        u32 normal_tex_idx;     // 4 bytes
        u32 roughness_tex_idx;  // 4 bytes
        u32 metalness_tex_idx;  // 4 bytes
};
