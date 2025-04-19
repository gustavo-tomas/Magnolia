#version 460

#include "text.include.glsl"

layout (location = 0) in vec2 in_tex_coords;
layout (location = 1) in vec4 in_color;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 texture_color = vec4(1.0, 1.0, 1.0, texture(u_char_texture, in_tex_coords).r);

    out_frag_color = texture_color * in_color;

    if (out_frag_color.a < 0.5) discard; // Discard transparent fragments
}
