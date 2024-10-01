// Check this (billboarding): https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/

#version 460

#include "sprite.include.glsl"

layout (location = 0) out vec2 out_tex_coords;

vec2 sprite_quad[6] = vec2[]
(
	vec2(-0.5,  0.5), vec2(-0.5, -0.5), vec2( 0.5,  0.5),
	vec2( 0.5,  0.5), vec2(-0.5, -0.5), vec2( 0.5, -0.5)
);

vec2 tex_coords[6] = vec2[]
(
	vec2(0, 0), vec2(0, 1), vec2(1, 0),
	vec2(1, 0), vec2(0, 1), vec2(1, 1)
);

void main()
{
	Sprite sprite = u_instance.sprites[gl_InstanceIndex];

	vec2 sprite_size = sprite.size_const_face.xy;
	float constant_size = sprite.size_const_face.z;
	float always_face_camera = sprite.size_const_face.w;

	vec3 position = vec3(sprite_quad[gl_VertexIndex] * sprite_size, 0);
	mat4 model_matrix = sprite.model;

	if (always_face_camera > 0)
	{
		vec3 camera_right = {VIEW_MATRIX[0][0], VIEW_MATRIX[1][0], VIEW_MATRIX[2][0]};
		vec3 camera_up = {VIEW_MATRIX[0][1], VIEW_MATRIX[1][1], VIEW_MATRIX[2][1]};
		vec3 sprite_center = vec3(model_matrix[3]);

		position = sprite_center + camera_right * position.x + camera_up * position.y;
	}

	gl_Position = PROJ_MATRIX * VIEW_MATRIX * model_matrix * vec4(position, 1.0);
	out_tex_coords = tex_coords[gl_VertexIndex];
}
