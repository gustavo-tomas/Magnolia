#version 460

#include "include/common.h"
#include "include/phong.glsl"

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;

	// Lights
	uint light_count;
} u_global;

// Light buffer
layout (std140, set = 0, binding = 1) readonly buffer LightBuffer
{
    LightData lights[];
} u_light;

#define VIEW_MATRIX u_global.view
#define PROJ_MATRIX u_global.projection

#ifdef VERTEX_SHADER

layout (location = 0) out vec3 out_ray_dir;

const vec2 grid_plane[] = vec2[]
(
    vec2(1, -1), vec2(1, 1), vec2(-1, -1), vec2(-1, 1)
);

void main() 
{
    vec2 position = grid_plane[gl_VertexIndex].xy;

    // Calculate ray direction directly
    vec4 clip_pos = vec4(position, 1.0, 1.0);
    vec4 view_pos = inverse(PROJ_MATRIX) * clip_pos;
    view_pos.xyz /= view_pos.w;
    
    // Transform to world space direction (don't add camera position yet)
    out_ray_dir = (inverse(VIEW_MATRIX) * vec4(normalize(view_pos.xyz), 0.0)).xyz;

    gl_Position = vec4(position, 0.0, 1.0); 
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) in vec3 in_ray_dir;

layout (location = 0) out vec4 out_frag_color;

void main()
{
    vec3 camera_position = vec3(inverse(VIEW_MATRIX)[3]);
    vec3 ray_dir = normalize(in_ray_dir);
    
    // Intersect with floor plane (y = 0)
    float t = -camera_position.y / ray_dir.y;
    
    if (t > 0.0)
    {
        vec3 hit_point = camera_position + t * ray_dir;
        vec2 uv = hit_point.xz * 0.04; // Scale factor
        vec3 normal = vec3(0.0, 1.0, 0.0); // Point up

        // Calculate proper depth
        vec4 clip_pos = PROJ_MATRIX * VIEW_MATRIX * vec4(hit_point, 1.0);
        float depth = clip_pos.z / clip_pos.w;
        gl_FragDepth = depth;
        
        // Checkerboard pattern
        vec2 checker = floor(uv);
        float pattern = mod(checker.x + checker.y, 2.0);
        vec3 color = mix(vec3(0.3), vec3(0.9), pattern);

        // z axis
        if (hit_point.x > -1 && hit_point.x < 1)
        {
            color = vec3(0.3, 0.3, 0.9);
        }
    
        // x axis
        if (hit_point.z > -1 && hit_point.z < 1)
        {
            color = vec3(0.9, 0.3, 0.3);
        }
        
        // Add some distance fog
        float fog = exp(-t * 0.0025);
        color = mix(vec3(0.5), color, fog);
        
        // Lighting

        out_frag_color = vec4(0.0, 0.0, 0.0, 1.0);

        for (uint i = 0; i < u_global.light_count; i++)
        {	
            PhongLight light;
            light.position = u_light.lights[i].position;
            light.color = u_light.lights[i].color;
            light.intensity = u_light.lights[i].intensity;

            vec3 lighting_color = phong_shading(normal, hit_point, camera_position, 0.0, light);
            out_frag_color.rgb += color.rgb * lighting_color;
        }
	    
        out_frag_color = clamp(out_frag_color, vec4(0.0), vec4(1.0));
    }
    
    else
    {
        discard;
    }
}

#endif
