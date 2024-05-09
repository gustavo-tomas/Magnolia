#version 460

#include "include/types.hglsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coords;
layout (location = 2) out vec3 out_frag_position;

void main()
{
	gl_Position = u_camera.projection * u_camera.view * u_model.model * vec4(in_position, 1.0);
	out_frag_position = vec3(u_model.model * vec4(in_position, 1.0));
	out_normal = mat3(transpose(inverse(u_model.model))) * in_normal;
	out_tex_coords = in_tex_coords;
}
