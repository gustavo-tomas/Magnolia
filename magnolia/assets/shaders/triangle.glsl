#version 460

/* Compile with

glslc -Imagnolia/assets/shaders
      -DVERTEX_SHADER
	  -fshader-stage=vertex
	  magnolia/assets/shaders/triangle.glsl
	  -o build/linux/debug/bin/shaders/triangle.vert.spv

	  and swap 'vert' for 'frag' or any other shader stage
*/

#ifdef VERTEX_SHADER

layout (location = 0) out vec4 out_color;

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

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	out_color = colors[gl_VertexIndex];
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec4 in_color;

layout (location = 0) out vec4 out_frag_color;

void main()
{
    out_frag_color = in_color;
}

#endif
