#version 460

#include "include/types.glsl"

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = vec4(in_color, 1.0);
}
