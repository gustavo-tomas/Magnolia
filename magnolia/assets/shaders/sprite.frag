#version 460

#include "sprite.include.glsl"

layout (location = 0) in vec2 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	vec4 object_color = texture(u_sprite_texture, in_tex_coords);

    out_frag_color = object_color;

    if (out_frag_color.a < 0.5) discard; // Discard transparent fragments
}
