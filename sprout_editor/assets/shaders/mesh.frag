#version 460

#include "mesh.include.glsl"

#include "include/utils.glsl"
#include "include/depth.glsl"
#include "include/pbr.glsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coords;
layout (location = 2) in vec3 in_frag_position;
layout (location = 3) in mat3 in_tbn;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 object_color = texture(ALBEDO_TEXTURE, in_tex_coords);
	vec4 object_normal = texture(NORMAL_TEXTURE, in_tex_coords);
	vec4 object_roughness = texture(ROUGHNESS_TEXTURE, in_tex_coords);
	vec4 object_metalness = texture(METALNESS_TEXTURE, in_tex_coords);

	// @TODO: we are loading all images (including normal maps) in the srgb format, but they should be
	// loaded with the linear format. In the future, there should be a import or json file describing
	// information about an images color space and other relevant data
	object_normal.rgb = linear_to_srgb(object_normal.rgb);
	object_roughness.rgb = linear_to_srgb(object_roughness.rgb);
	object_metalness.rgb = linear_to_srgb(object_metalness.rgb);

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

	// PBR shading
	vec4 pbr_color = vec4(0);
	
	MaterialData material = u_material.materials[u_push_constants.material_index];
	material.albedo *= object_color;
	material.roughness *= object_roughness.g;
	material.metallic *= object_metalness.b;

	pbr_color.rgb = pbr_shading(material,
								normal,
								camera_position,
								in_frag_position,
								u_push_constants.texture_output);

	pbr_color.a = object_color.a;

	// Texture Outputs: 0 - "Combined", 1 - "Albedo", 2 - "Normal", 3 - "Roughness", 4 - "Metalness"
	// PBR Outputs:     5 - 8
	switch (u_push_constants.texture_output)
	{
		// Final result
		case 0:
			out_frag_color = pbr_color;
			if (out_frag_color.a < 0.5) discard; // Discard transparent fragments
			break;

		// Final normal
		case 1:
			normal = normal * 0.5 + 0.5; // Normalize from [-1, 1] to [0, 1]
			out_frag_color = vec4(normal, 1.0);
			break;
		
		// Depth
		case 2:
			float near = u_global.near_far.x;
			float far = u_global.near_far.y;
			out_frag_color = vec4(depth_to_color(gl_FragCoord.z, near, far), 1.0);
			break;
		
		// Albedo
		case 3:
			out_frag_color = vec4(object_color.rgb, object_color.a);
			break;

		// Normal map
		case 4:
			out_frag_color = object_normal;
			break;

		// Roughness
		case 5:
			out_frag_color = vec4(object_roughness.g);
			break;
			
		// Metalness
		case 6:
			out_frag_color = vec4(object_metalness.b);
			break;

		default:
			out_frag_color = pbr_color;
			break;
	}
}
