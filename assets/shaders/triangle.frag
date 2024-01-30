#version 460

layout (location = 0) in vec3 in_normal;

layout (location = 0) out vec4 out_frag_color;

void main()
{
	out_frag_color = vec4(in_normal, 1.0);
}
