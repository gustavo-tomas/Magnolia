#version 460

#include "include/common.h"
#include "include/phong.glsl"
#include "include/utils.glsl"

// Based on this implementation from simondev: https://www.youtube.com/watch?v=bp7REZBV4P4

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
   GlobalGrassData data;
} u_global;

// Instance buffer
layout (std140, set = 0, binding = 1) readonly buffer InstanceBuffer
{
    GrassData blades[];
} u_instance;

// Light buffer
layout (std140, set = 0, binding = 2) readonly buffer LightBuffer
{
    LightData lights[];
} u_light;

#define VIEW_MATRIX      u_global.data.camera_data.view
#define PROJ_MATRIX      u_global.data.camera_data.projection
#define MAX_BLADE_HEIGHT u_global.data.max_blade_height

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec3 out_frag_position;
layout (location = 3) out vec3 out_original_vertex_position;

struct Gradient
{
        float cutoff;
        vec3 color;
};

Gradient green_gradients[] = {
    { 0.7,  vec3(0.925, 0.952, 0.619) },
    { 0.5,  vec3(0.564, 0.662, 0.333) },
    { 0.2,  vec3(0.309, 0.466, 0.176) },
    { 0.15, vec3(0.192, 0.341, 0.172) },
    { 0.0,  vec3(0.074, 0.164, 0.074) }
};

Gradient cold_gradients[] = {
    { 0.5,  vec3(0.741, 0.910, 0.961) },
    { 0.3,  vec3(0.286, 0.533, 0.769) },
    { 0.1,  vec3(0.110, 0.302, 0.553) },
    { 0.0,  vec3(0.059, 0.157, 0.329) },
};

vec3 color_gradient(float y_pos)
{
    uint size = green_gradients.length();
    for (uint i = 0; i < size; i++)
    {
        float cutoff = green_gradients[i].cutoff;
        vec3 color = green_gradients[i].color;

        if (y_pos >= MAX_BLADE_HEIGHT * cutoff)
        {
            return color;
        }
    }
}

float calculate_rotation_y(vec3 grass_blade_position)
{
    float angle = perlin_noise(grass_blade_position.xz) * PI;
    // float angle = perlin_noise_normalized(grass_blade_position.xz) * PI;

    return angle;
}

float calculate_rotation_z(vec3 grass_blade_position, float speed, float strength)
{
    float angle = 0.15;
    
    float height_factor = in_position.y / MAX_BLADE_HEIGHT;
    angle *= height_factor;

    float noise = perlin_noise_normalized(vec2(u_global.data.time * speed) + grass_blade_position.xz) * strength;
    angle += noise;

    return angle;
}

float calculate_wind_strength(vec3 grass_blade_position, float speed, float range)
{
    float wind_strength = perlin_noise_normalized(grass_blade_position.xz * range + u_global.data.time * speed);

    return wind_strength;
}

void main()
{
    // @TODO: fix normals and finish normal matrix

    vec3 vertex_position = in_position;
    vec3 grass_blade_position = u_instance.blades[gl_InstanceIndex].position;
   
    // Calculate grass blade rotation around Z and Y axes

    float blade_speed = 0.0005;
    float blade_strength = 0.12;
    float wind_speed = 0.0007;
    float wind_range = -0.03;
    float wind_angle = HALF_PI;

    float rotation_z = calculate_rotation_z(grass_blade_position, blade_speed, blade_strength);
    vertex_position = rotate(vertex_position, vec3(0, 0, 1), rotation_z);

    float rotation_y = calculate_rotation_y(grass_blade_position);
    vertex_position = rotate(vertex_position, vec3(0, 1, 0), rotation_y);

    float wind_strength = calculate_wind_strength(grass_blade_position, wind_speed, wind_range) * 0.95;
    vertex_position = rotate(vertex_position, vec3(0, 0, 1), wind_angle * wind_strength);

	out_frag_position = vertex_position + grass_blade_position;
	out_normal = in_normal;
    out_color = color_gradient(in_position.y);
	out_original_vertex_position = in_position;

	gl_Position = PROJ_MATRIX * VIEW_MATRIX * vec4(out_frag_position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_frag_position;
layout (location = 3) in vec3 in_original_vertex_position;

layout (location = 0) out vec4 out_frag_color;

// @TODO: better ao (use density with height instead of only height)

void main()
{
    // @TODO: Probably shouldn't be in the frag shader
	vec3 camera_position = vec3(inverse(VIEW_MATRIX)[3]);

    out_frag_color = vec4(0.0, 0.0, 0.0, 1.0);

    float height_factor = in_original_vertex_position.y / MAX_BLADE_HEIGHT;
    float ao = (1.0 - height_factor) * 0.75;

    for (uint i = 0; i < u_global.data.light_count; i++)
	{	
		PhongLight light;
		light.position = u_light.lights[i].position;
		light.color = u_light.lights[i].color;
		light.intensity = u_light.lights[i].intensity;

		vec3 lighting_color = phong_shading(in_normal, in_frag_position, camera_position, ao, light);
		out_frag_color.rgb += in_color.rgb * lighting_color;
		// out_frag_color.rgb += lighting_color;
	}

    // Debug
	// out_frag_color = vec4(in_color, 1.0);
	// out_frag_color = vec4(in_normal, 1.0);
	// out_frag_color = vec4(height_factor, height_factor, height_factor, 1.0);
	// out_frag_color = vec4(ao, ao, ao, 1.0);

	out_frag_color = clamp(out_frag_color, vec4(0.0), vec4(1.0));
}

#endif
