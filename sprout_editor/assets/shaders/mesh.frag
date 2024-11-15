#version 460

#include "mesh.include.glsl"
#include "include/utils.glsl"
#include "include/tonemapping.glsl"
#include "include/phong.glsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coords;
layout (location = 2) in vec3 in_frag_position;
layout (location = 3) in mat3 in_tbn;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 object_color = texture(ALBEDO_TEXTURE, in_tex_coords);
	vec4 object_normal = texture(NORMAL_TEXTURE, in_tex_coords);

	// @TODO: we are loading all images (including normal maps) in the srgb format, but they should be
	// loaded with the linear format. In the future, there should be a import or json file describing
	// information about an images color space and other relevant data
	object_normal.rgb = linear_to_srgb(object_normal.rgb);

	// @TODO: this is pretty slow, but for now its ok
	vec3 camera_position = vec3(inverse(VIEW_MATRIX)[3]);

	// Select the fragment normal
	vec3 normal = vec3(0);
	if (u_push_constants.normal_output == 0)
	{
		normal = in_normal;
	}

	else
	{
		normal = calculate_normals_from_normal_map(object_normal.rgb, in_normal, in_frag_position, in_tex_coords);

		// @TODO: normal calculation generate some artifacts
		// normal = normalize(object_normal.rgb * 2.0 - 1.0);
		// normal = normalize(in_tbn * normal);
	}

	// Phong shading
	vec3 phong_color = vec3(0);
	for (uint i = 0; i < u_push_constants.number_of_lights; i++)
	{
		phong_color += phong_shading(normal, in_frag_position, camera_position, u_lights.lights[i]);
	}

	switch (u_push_constants.texture_output)
	{
		case 0:
			out_frag_color = vec4(phong_color * object_color.rgb, object_color.a);
			if (out_frag_color.a < 0.5) discard; // Discard transparent fragments
			break;
		
		case 1:
			out_frag_color = vec4(object_color.rgb, object_color.a);
			break;

		case 2:
			out_frag_color = vec4(normal, 1.0);
			break;
		
		case 3:
			out_frag_color = vec4(phong_color, 1.0);
			break;

		default:
			out_frag_color = vec4(1.0);
			break;
	}
}
