#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

layout (location = 0) out vec3 out_normal;

void main()
{
	gl_Position = vec4(in_position, 1.0);
	out_normal = in_normal;
}
