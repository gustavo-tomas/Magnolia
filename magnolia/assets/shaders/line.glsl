#version 460

#include "include/common.h"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
} u_global;

#define VIEW_MATRIX u_global.view
#define PROJ_MATRIX u_global.projection

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(in_position, 1.0);
	out_color = in_color;
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = vec4(in_color, 1.0);
}

#endif
