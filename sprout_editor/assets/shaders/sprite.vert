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

	vec3 position = vec3(sprite_quad[gl_VertexIndex] * sprite.size, 0);
	mat4 model_matrix = sprite.model;

	gl_Position = PROJ_MATRIX * VIEW_MATRIX * model_matrix * vec4(position, 1.0);
	out_tex_coords = tex_coords[gl_VertexIndex];
}
