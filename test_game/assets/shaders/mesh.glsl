#version 460

#include "include/common.h"
#include "include/phong.glsl"

// @TODO: move to material
uint normal_output = 0;

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    CameraData camera;
	uint light_count;
} u_global;

// Model buffer
layout (std140, set = 0, binding = 1) readonly buffer ModelBuffer
{
	ModelData models[];
} u_model;

// Instance buffer
layout (std140, set = 0, binding = 2) readonly buffer InstanceBuffer
{
    MeshData meshes[];
} u_instance;

// Light buffer
layout (std140, set = 0, binding = 3) readonly buffer LightBuffer
{
    LightData lights[];
} u_light;

// Material buffer
layout (std140, set = 0, binding = 4) readonly buffer MaterialBuffer
{
    MaterialData materials[];
} u_material;

// Material textures
layout (set = 0, binding = 5) uniform sampler2D u_material_textures[];

#define VIEW_MATRIX u_global.camera.view
#define PROJ_MATRIX u_global.camera.projection

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coords;
layout (location = 2) out vec3 out_frag_position;
layout (location = 3) out flat uint out_material_idx;
layout (location = 4) out mat3 out_tbn;

void main()
{
	uint model_idx = u_instance.meshes[gl_InstanceIndex].model_idx;
	uint material_idx = u_instance.meshes[gl_InstanceIndex].material_idx;
	mat4 model_matrix = u_model.models[model_idx].model;

	gl_Position = PROJ_MATRIX * VIEW_MATRIX * model_matrix * vec4(in_position, 1.0);
	out_frag_position = vec3(model_matrix * vec4(in_position, 1.0));
	out_tex_coords = in_tex_coords;
	out_material_idx = material_idx;

	// @TODO: this is pretty slow, but for now its ok
	// Multiply normal by the normal matrix to avoid problems with non uniform scaling 
	mat3 normal_matrix = mat3(transpose(inverse(model_matrix)));
	out_normal = normalize(normal_matrix * in_normal);

	vec3 T = normalize(normal_matrix * in_tangent);
	vec3 N = out_normal;
	
	// Re-orthogonalize T with respect to N to prevent orthogonalization errors on larger meshes
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);

	// Check if the TBN is in a right-handed coordinate system
	if (dot(cross(N, T), B) < 0.0) T = T * -1.0;

	out_tbn = TBN;
}

#endif

#ifdef FRAGMENT_SHADER

#include "include/utils.glsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coords;
layout (location = 2) in vec3 in_frag_position;
layout (location = 3) in flat uint in_material_idx;
layout (location = 4) in mat3 in_tbn;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	MaterialData material = u_material.materials[in_material_idx];

	vec4 object_color = texture(u_material_textures[material.albedo_tex_idx], in_tex_coords);
	vec4 object_normal = texture(u_material_textures[material.normal_tex_idx], in_tex_coords);
	vec4 object_roughness = texture(u_material_textures[material.roughness_tex_idx], in_tex_coords);
	vec4 object_metalness = texture(u_material_textures[material.metalness_tex_idx], in_tex_coords);

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
	if (normal_output == 0)
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

	// Lighting

	out_frag_color = vec4(0.0, 0.0, 0.0, object_color.a);

	for (uint i = 0; i < u_global.light_count; i++)
	{	
		PhongLight light;
		light.position = u_light.lights[i].position;
		light.color = u_light.lights[i].color;
		light.intensity = u_light.lights[i].intensity;

		float ao = 0;

		vec3 lighting_color = phong_shading(in_normal, in_frag_position, camera_position, ao, light);
		out_frag_color.rgb += object_color.rgb * lighting_color;
	}

	out_frag_color = clamp(out_frag_color, vec4(0.0), vec4(1.0));

	if (out_frag_color.a < 0.5)
	{
		// Discard transparent fragments
		discard;
	}
}

#endif
