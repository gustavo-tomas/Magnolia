#version 460

#include "post.include.glsl"

#include "include/tonemapping.glsl"

layout (location = 0) in vec2 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

void main()
{	
	out_frag_color = texture(u_screen_color_texture, in_tex_coords);

	if (u_push_constants.apply_tonemapping)
	{
		out_frag_color.rgb = tonemapping_reinhard(out_frag_color.rgb);
	}
	
	// Gamma correct (done during presentation)
	// out_frag_color.rgb = pow(out_frag_color.rgb, vec3(0.4545));
}
