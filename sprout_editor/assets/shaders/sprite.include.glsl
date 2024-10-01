#include "include/types.glsl"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 screen_size;
} u_global;

struct Sprite
{
    mat4 model;
    vec2 size;
};

// Instance buffer
layout (std140, set = 1, binding = 0) readonly buffer InstanceBuffer
{
    Sprite sprites[];
} u_instance;

// Texture
layout (set = 2, binding = 0) uniform sampler2D u_sprite_texture;
