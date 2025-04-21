#version 460

#include "text.include.glsl"

layout (location = 0) out vec2 out_tex_coords;
layout (location = 1) out vec4 out_color;

vec3 quad[4] = vec3[]
(
	vec3(0, 1, 0), vec3(0, 0, 0), vec3(1, 1, 0), vec3(1, 0, 0)
);

vec2 tex_coords[4] = vec2[]
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
}
