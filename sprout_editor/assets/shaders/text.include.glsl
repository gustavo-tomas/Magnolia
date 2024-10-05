#include "include/types.glsl"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
} u_global;

// Texture
layout (set = 1, binding = 0) uniform sampler2D u_atlas_texture;
layout (set = 2, binding = 0) uniform TextInfoBuffer
{
    vec4 color;
    float pixel_range;
} u_text_info;
