#version 460

/* Compile with

glslc -Imagnolia/assets/shaders
      -DVERTEX_SHADER
	  -fshader-stage=vertex
	  magnolia/assets/shaders/triangle.glsl
	  -o build/linux/debug/bin/shaders/triangle.vert.spv

	  and swap 'vert' for 'frag' or any other shader stage
*/

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    mat4 view;
    mat4 projection;
    mat4 model;
} u_global;

layout (set = 0, binding = 1) uniform sampler2D u_textures[];

#ifdef VERTEX_SHADER

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_tex_coords;

const vec3 positions[] = vec3[3]
(
	vec3( 0.5,  0.5, 0.0),
	vec3(-0.5,  0.5, 0.0),
	vec3( 0.0, -0.5, 0.0)
);

const vec4 colors[] = vec4[3]
(
	vec4(1.0, 0.0, 0.0, 1.0), 
	vec4(0.0, 1.0, 0.0, 1.0), 
	vec4(0.0, 0.0, 1.0, 1.0)  
);

const vec2 tex_coords[] = vec2[]
(
	vec2(0.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 0.0)
);

void main()
{
	gl_Position = u_global.projection * u_global.view * u_global.model * vec4(positions[gl_VertexIndex], 1.0);
	out_color = colors[gl_VertexIndex];
	out_tex_coords = tex_coords[gl_VertexIndex];
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

void main()
{
    out_frag_color = texture(u_textures[0], in_tex_coords);
    out_frag_color.a = 1.0;
}

#endif
