#ifdef __cplusplus

    #include "core/types.hpp"
    #include "math/mat.hpp"
    #include "math/types.hpp"
    #include "math/vec.hpp"

using namespace mag;
using namespace mag::math;

#else

    #define f32 float
    #define alignas(x)

// See this: https://developer.nvidia.com/vulkan-shader-resource-binding

// Macros
    #define MODEL_MATRIX u_instance.models[gl_InstanceIndex].model
    #define PROJ_MATRIX u_global.projection
    #define VIEW_MATRIX u_global.view
    #define NEAR_FAR u_global.near_far

// Constants
const float PI = 3.1415926535;

#endif

// Types shared by c++ and glsl

struct alignas(16) ModelData
{
        mat4 model;  // 64 bytes (16 x 4)
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
};

struct alignas(16) MaterialData
{
        vec4 albedo;    // 16 bytes
        f32 roughness;  // 4 bytes
        f32 metallic;   // 4 bytes
};
