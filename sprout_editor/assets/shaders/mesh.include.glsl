#include "include/types.glsl"

// @TODO: for now, only fragment shaders support push constants
// Push constants (check size limits! -> 128)
layout (push_constant) uniform PushConstants
{
    // Shader debug parameters
    uint texture_output;
    uint normal_output;

    // Scene parameters
    uint number_of_lights;
} u_push_constants;

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
    Light lights[];
} u_lights;

// Instance buffer
layout (std140, set = 2, binding = 0) readonly buffer InstanceBuffer
{
    Model models[];
} u_instance;

// Materials
// 0 - Albedo | 1 - Normal
layout (set = 3, binding = 0) uniform sampler2D u_material_textures[2];
