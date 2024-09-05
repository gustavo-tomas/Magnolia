#include "include/types.glsl"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 near_far;
} u_global;

// Lights buffer
layout (std140, set = 1, binding = 0) readonly buffer LightBuffer
{
    uint number_of_lights;
    Light lights[];
} u_lights;

// Instance buffer
layout (std140, set = 2, binding = 0) readonly buffer InstanceBuffer
{
    Model models[];
} u_instance;

// Shader parameters
layout (set = 3, binding = 0) uniform ShaderBuffer
{
    // Debug
    uint texture_output;
    uint normal_output;
} u_shader;

// Materials
// 0 - Albedo | 1 - Normal
layout (set = 4, binding = 0) uniform sampler2D u_material_textures[2];
