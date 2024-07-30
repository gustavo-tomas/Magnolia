#version 460

#include "include/types.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(in_position, 1.0);
	out_color = in_color;
}
