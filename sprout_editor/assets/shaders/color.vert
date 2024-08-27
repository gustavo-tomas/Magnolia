#version 460

#include "include/types.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
} u_global;

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(in_position, 1.0);
	out_color = in_color;
}
