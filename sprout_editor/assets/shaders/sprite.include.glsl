#include "include/common.h"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 screen_size;
} u_global;

// Instance buffer
layout (std140, set = 1, binding = 0) readonly buffer InstanceBuffer
{
    SpriteData sprites[];
} u_instance;

// Texture
layout (set = 2, binding = 0) uniform sampler2D u_sprite_texture;
