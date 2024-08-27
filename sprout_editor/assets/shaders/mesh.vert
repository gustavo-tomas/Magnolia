#version 460

#include "mesh.include.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coords;
layout (location = 2) out vec3 out_frag_position;
layout (location = 3) out mat3 out_tbn;

void main()
{
	gl_Position = PROJ_MATRIX * VIEW_MATRIX * MODEL_MATRIX * vec4(in_position, 1.0);
	out_frag_position = vec3(MODEL_MATRIX * vec4(in_position, 1.0));
	out_tex_coords = in_tex_coords;
	
	// @TODO: this is pretty slow, but for now its ok
	// Multiply normal by the normal matrix to avoid problems with non uniform scaling 
	mat3 normal_matrix = mat3(transpose(inverse(MODEL_MATRIX)));
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
