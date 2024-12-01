// Convert a depth value to NDC space
float linearize_depth(float depth, float near, float far)
{
    return (2.0 * near * far) / (far + near - depth * (far - near));
}

// Normalize depth for color mapping
float normalize_depth(float depth, float near, float far)
{
    return linearize_depth(depth, near, far) / (far - near);
}

// Convert a depth value to a color range
vec3 depth_to_color(float depth, float near, float far)
{
    // Rainbow colormap
    vec3 colors[5] = vec3[]
    (
        vec3(0.0, 0.0, 1.0),   // Blue (closest)
        vec3(0.0, 1.0, 1.0),   // Cyan
        vec3(0.0, 1.0, 0.0),   // Green
        vec3(1.0, 1.0, 0.0),   // Yellow
        vec3(1.0, 0.0, 0.0)    // Red (furthest)
    );
    
    // Interpolate between color bands
    float scaled_depth = normalize_depth(depth, near, far) * 4.0;
    int index = int(scaled_depth);
    float t = fract(scaled_depth);
    
    return mix(colors[index], colors[index + 1], t);
}
