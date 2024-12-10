#version 460

#include "depth_prepass.include.glsl"

layout (location = 0) in vec3 in_position;

// @TODO: these are just to match the mesh vertex layout
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;
// @TODO: these are just to match the mesh vertex layout

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(in_position, 1.0);
}
