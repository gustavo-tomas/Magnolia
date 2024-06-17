#version 460

#include "include/types.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

// @TODO: unecessary vertex attributes
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

layout (location = 0) out vec3 out_color;

void main()
{
	gl_Position = u_global.projection * u_global.view * vec4(in_position, 1.0);
	out_color = in_color;
}
