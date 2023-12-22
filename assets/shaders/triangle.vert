#version 460

layout (location = 0) out vec3 color;

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3( 0.5,  0.5, 0.0),
		vec3(-0.5,  0.5, 0.0),
		vec3( 0.0, -0.5, 0.0)
	);

	// const array of colors for the triangle
	const vec3 colors[3] = vec3[3](
		vec3(1.0, 0.0, 0.0), //red
		vec3(0.0, 1.0, 0.0), //green
		vec3(0.0, 0.0, 1.0)  //blue
	);

	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	color = colors[gl_VertexIndex];
}
