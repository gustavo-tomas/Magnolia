#version 460

#include "types.hglsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	int tex_idx = int(in_tex_coords.z);
	out_frag_color = vec4(in_normal, 1.0);
	// out_frag_color = texture(u_diffuse_textures[tex_idx], in_tex_coords.xy);
}
