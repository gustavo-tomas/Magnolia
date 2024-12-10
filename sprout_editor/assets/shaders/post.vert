#version 460

#include "post.include.glsl"

layout (location = 0) out vec2 out_tex_coords;

vec3 grid_plane[6] = vec3[]
(
    vec3( 1, -1, 0), vec3(-1,  1, 0), vec3(-1, -1, 0),
    vec3(-1,  1, 0), vec3( 1, -1, 0), vec3( 1,  1, 0)
);

vec2 grid_tex_coords[6] = vec2[]
(
    vec2(1, 1), vec2(0, 0), vec2(0, 1),
    vec2(0, 0), vec2(1, 1), vec2(1, 0)
);

void main()
{
    vec3 p = grid_plane[gl_VertexIndex].xyz;
	gl_Position = vec4(p, 1.0);

	out_tex_coords = grid_tex_coords[gl_VertexIndex];
}
