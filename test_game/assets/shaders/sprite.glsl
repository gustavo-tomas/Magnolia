// Check this (billboarding): https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/

#version 460

#include "include/common.h"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    CameraData camera;
} u_global;

// Instance buffer
layout (std140, set = 0, binding = 1) readonly buffer InstanceBuffer
{
    SpriteData sprites[];
} u_instance;

// Textures
layout (set = 0, binding = 2) uniform sampler2D u_sprite_textures[];

#ifdef VERTEX_SHADER

layout (location = 0) out vec2 out_tex_coords;
layout (location = 1) out flat uint out_tex_idx;

const vec2 sprite_quad[] = vec2[]
(
	vec2(-0.5, 0.5), vec2(-0.5, -0.5), vec2(0.5, 0.5), vec2(0.5, -0.5)
);

const vec2 tex_coords[] = vec2[]
(
	vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

void main()
{
	const mat4 VIEW_MATRIX = u_global.camera.view;
	const mat4 PROJ_MATRIX = u_global.camera.projection;

	SpriteData sprite = u_instance.sprites[gl_InstanceIndex];

	vec2 sprite_size = sprite.size_const_face.xy;
	float constant_size = sprite.size_const_face.z;
	float always_face_camera = sprite.size_const_face.w;

	vec3 position = vec3(sprite_quad[gl_VertexIndex] * sprite_size, 0);
	mat4 model_matrix = sprite.model;
	vec3 sprite_center = vec3(model_matrix[3]);

	// Scale the sprite as the distance increases
	if (constant_size == 1)
	{
		vec3 camera_position = vec3(inverse(VIEW_MATRIX)[3]);

		float distance = length(camera_position - sprite_center);

		vec2 scale = vec2(length(vec3(model_matrix[0])), length(vec3(model_matrix[1])));

		vec2 scaled_size = sprite_size * distance * scale;

		position = vec3(sprite_quad[gl_VertexIndex] * scaled_size, 0);
	}

	// Align the position to the camera vectors
	if (always_face_camera == 1)
	{
		vec3 camera_right = {VIEW_MATRIX[0][0], VIEW_MATRIX[1][0], VIEW_MATRIX[2][0]};
		vec3 camera_up = {VIEW_MATRIX[0][1], VIEW_MATRIX[1][1], VIEW_MATRIX[2][1]};

		position = camera_right * position.x + camera_up * position.y;

		// @NOTE: We remove the model rotation in the c++ side
	}

	gl_Position = PROJ_MATRIX * VIEW_MATRIX * model_matrix * vec4(position, 1.0);
	out_tex_coords = tex_coords[gl_VertexIndex];
	out_tex_idx = sprite.texture_idx;
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec2 in_tex_coords;
layout (location = 1) in flat uint in_tex_idx;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 object_color = texture(u_sprite_textures[nonuniformEXT(in_tex_idx)], in_tex_coords);

    out_frag_color = object_color;

	// Discard transparent fragments
    if (out_frag_color.a < 0.5)
	{
		discard;
	}
}

#endif
