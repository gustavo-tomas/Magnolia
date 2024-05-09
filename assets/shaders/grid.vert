#version 460

#include "include/types.hglsl"

layout (location = 0) out vec2 out_near_far;
layout (location = 1) out vec3 out_near_point;
layout (location = 2) out vec3 out_far_point;
layout (location = 3) out mat4 out_view;
layout (location = 7) out mat4 out_projection;

// Grid position are in xy clipped space
vec3 grid_plane[6] = vec3[]
(
    vec3( 1,  1, 0), vec3(-1, -1, 0), vec3(-1,  1, 0),
    vec3(-1, -1, 0), vec3( 1,  1, 0), vec3( 1, -1, 0)
);

vec3 unproject_point(float x, float y, float z, mat4 view, mat4 projection) 
{
    vec4 unprojected_point =  inverse(view) * inverse(projection) * vec4(x, y, z, 1.0);
    return unprojected_point.xyz / unprojected_point.w;
}

void main() 
{
    vec3 p = grid_plane[gl_VertexIndex].xyz;
    out_near_point = unproject_point(p.x, p.y, 0.0, u_camera.view, u_camera.projection).xyz;
    out_far_point = unproject_point(p.x, p.y, 1.0, u_camera.view, u_camera.projection).xyz;
    
    out_view = u_camera.view;
    out_projection = u_camera.projection;
    out_near_far = u_camera.near_far;

    gl_Position = vec4(p, 1.0); // using directly the clipped coordinates
}
