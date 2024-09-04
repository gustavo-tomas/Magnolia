#version 460

#include "sprite.include.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_tex_coords;

layout (location = 0) out vec2 out_tex_coords;

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(in_position, 1.0);
	out_tex_coords = in_tex_coords;
}
