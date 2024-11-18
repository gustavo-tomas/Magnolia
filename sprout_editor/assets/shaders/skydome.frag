#version 460

#include "skydome.include.glsl"

layout (location = 0) in float in_height;
layout (location = 1) in vec2 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	// Use this and/or variations to create a nice horizon gradient
	vec3 horizon_color = vec3(0.5, 0.6, 0.9), top_color = vec3(0.5, 0.8, 0.95);
	vec3 skydome_color = mix(horizon_color, top_color, in_height);

	out_frag_color = vec4(skydome_color, 1.0);
}
