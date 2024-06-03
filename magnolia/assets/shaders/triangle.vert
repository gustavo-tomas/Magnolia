#version 460

#include "include/types.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coords;
layout (location = 2) out vec3 out_frag_position;

void main()
{
	gl_Position = u_global.projection * u_global.view * u_instance.model * vec4(in_position, 1.0);
	out_frag_position = vec3(u_instance.model * vec4(in_position, 1.0));
	out_tex_coords = in_tex_coords;
	
	// @TODO: this is pretty slow, but for now its ok
	out_normal = normalize(mat3(transpose(inverse(u_instance.model))) * in_normal);
}
