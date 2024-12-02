#include "include/common.glsl"

// Types
struct Model
{
	mat4 model;
};

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 near_far;
} u_global;

// Instance buffer
layout (std140, set = 1, binding = 0) readonly buffer InstanceBuffer
{
    Model models[];
} u_instance;
