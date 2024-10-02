#version 460

#include "grid.include.glsl"

layout (location = 0) in vec3 in_near_point;
layout (location = 1) in vec3 in_far_point;

layout (location = 0) out vec4 out_color;

vec4 grid(vec3 frag_pos_3D, float scale)
{
    vec2 coord = frag_pos_3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float min_z = min(derivative.y, 1);
    float min_x = min(derivative.x, 1);
    vec4 color = vec4(0.025, 0.025, 0.025, 1.0 - min(line, 1.0));
    
    // z axis
    if (frag_pos_3D.x > -1 * min_x && frag_pos_3D.x < 1 * min_x)
        color.z = 1.0;
    
    // x axis
    if (frag_pos_3D.z > -1 * min_z && frag_pos_3D.z < 1 * min_z)
        color.x = 1.0;
    
    return color;
}

float compute_depth(vec3 pos)
{
    vec4 clip_space_pos = PROJ_MATRIX * VIEW_MATRIX * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

float compute_linear_depth(vec3 pos)
{
    float near = NEAR_FAR.x;
    float far = NEAR_FAR.y;
    vec4 clip_space_pos = PROJ_MATRIX * VIEW_MATRIX * vec4(pos.xyz, 1.0);

    // put back between -1 and 1
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;

    // get linear value between near and far
    float linear_depth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); 
    
    return linear_depth / far; // normalize
}

void main()
{
    float t = -in_near_point.y / (in_far_point.y - in_near_point.y);

    if (t < 0) discard; // Discard fragments above the grid

    vec3 frag_pos_3D = in_near_point + t * (in_far_point - in_near_point);
    gl_FragDepth = compute_depth(frag_pos_3D);
    
    float linear_depth = compute_linear_depth(frag_pos_3D) * 6;
    float fading = max(0, (0.5 - linear_depth));

    out_color = grid(frag_pos_3D, 0.05) + grid(frag_pos_3D, 0.5); // adding multiple resolution for the grid
    out_color.a *= fading;

    if (out_color.a > 0.999) discard; // Make a smooth horizon gradient
}
