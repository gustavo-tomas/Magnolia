#version 460

#include "include/types.glsl"
#include "include/phong.glsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coords;
layout (location = 2) in vec3 in_frag_position;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 object_color = texture(u_diffuse_texture, in_tex_coords);
	if (object_color.a < 0.5) discard;

	// @TODO: this is pretty slow, but for now its ok
	vec3 camera_position = vec3(inverse(u_global.view)[3]);

	// Phong shading
	vec3 phong_color = vec3(0);
	for (int i = 0; i < MAX_NUMBER_OF_LIGHTS; i++)
	{
		phong_color += phong_shading(in_normal, in_frag_position, camera_position, u_global.point_lights[i]);
	}

	out_frag_color = vec4(phong_color * object_color.rgb, object_color.a);
}