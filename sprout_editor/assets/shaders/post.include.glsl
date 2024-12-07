#include "include/common.glsl"

// Push constants (dont exceed 128 bytes)
layout (push_constant) uniform PushConstants
{
    // Shader debug parameters
    bool apply_tonemapping;
} u_push_constants;

// Texture
layout (set = 0, binding = 0) uniform sampler2D u_screen_color_texture;
