#version 460

#include "post.include.glsl"

layout (location = 0) out vec2 out_tex_coords;

const vec3 grid_plane[] = vec3[]
(
    vec3(-1.0, 1.0, 0.0), vec3(-1.0, -1.0, 0.0), vec3(1.0, 1.0, 0.0), vec3(1.0, -1.0, 0.0)
);

const vec2 grid_tex_coords[] = vec2[]
(
    vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

void main()
{
    vec3 p = grid_plane[gl_VertexIndex].xyz;
	gl_Position = vec4(p, 1.0);

	out_tex_coords = grid_tex_coords[gl_VertexIndex];
}
