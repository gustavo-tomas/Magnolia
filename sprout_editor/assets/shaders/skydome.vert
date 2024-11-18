#version 460

#include "skydome.include.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_tex_coords;

layout (location = 0) out float out_height;
layout (location = 1) out vec2 out_tex_coords;

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * u_global.world * vec4(in_position, 1.0);
	gl_Position.z = gl_Position.w;
	
	out_height = in_position.y;
	out_tex_coords = in_tex_coords;
}
