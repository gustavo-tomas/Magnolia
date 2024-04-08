#version 460

#include "types.hglsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = texture(u_diffuse_texture, in_tex_coords);
	if (out_frag_color.a < 0.5) discard;
}
