#version 460

#include "include/common.h"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
} u_global;

// Instance buffer
layout (std140, set = 0, binding = 1) readonly buffer InstanceBuffer
{
    TextData texts[];
} u_instance;

// Texture
layout (set = 0, binding = 2) uniform sampler2D u_char_textures[];

#define VIEW_MATRIX u_global.view
#define PROJ_MATRIX u_global.projection

#ifdef VERTEX_SHADER

layout (location = 0) out vec2 out_tex_coords;
layout (location = 1) out vec4 out_color;
layout (location = 2) out flat uint out_texture_idx;

const vec3 quad[] = vec3[]
(
	vec3(0, 1, 0), vec3(0, 0, 0), vec3(1, 1, 0), vec3(1, 0, 0)
);

const vec2 tex_coords[] = vec2[]
(
	vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

void main()
{
	TextData text = u_instance.texts[gl_InstanceIndex];

	vec3 position = quad[gl_VertexIndex];

	gl_Position = PROJ_MATRIX * VIEW_MATRIX * text.model * vec4(position, 1.0);
	out_tex_coords = tex_coords[gl_VertexIndex];
	out_color = text.color;
	out_texture_idx = text.texture_idx;
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec2 in_tex_coords;
layout (location = 1) in vec4 in_color;
layout (location = 2) in flat uint in_texture_idx;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 texture_color = vec4(1.0, 1.0, 1.0, texture(u_char_textures[in_texture_idx], in_tex_coords).r);

    out_frag_color = texture_color * in_color;

	// Discard transparent fragments
    if (out_frag_color.a < 0.5)
	{
		discard;
	}
}

#endif
