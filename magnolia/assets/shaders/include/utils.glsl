// Conversions, common calculations, etc

// https://gist.github.com/Reedbeta/e8d3817e3f64bba7104b8fafd62906df
vec3 srgb_to_linear(vec3 rgb)
{
    // See https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
    return mix(pow((rgb + 0.055) * (1.0 / 1.055), vec3(2.4)),
               rgb * (1.0/12.92),
               lessThanEqual(rgb, vec3(0.04045)));
}

vec3 linear_to_srgb(vec3 rgb)
{
    // See https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
    return mix(1.055 * pow(rgb, vec3(1.0 / 2.4)) - 0.055,
               rgb * 12.92,
               lessThanEqual(rgb, vec3(0.0031308)));
}

// Calculate tangent normals from a normal map
vec3 calculate_normals_from_normal_map(vec3 texture_normal, vec3 vertex_normal, vec3 frag_position, vec2 tex_coords)
{
    vec3 tangent_normal = texture_normal * 2.0 - 1.0;

    vec3 Q1  = dFdx(frag_position);
    vec3 Q2  = dFdy(frag_position);
    vec2 st1 = dFdx(tex_coords);
    vec2 st2 = dFdy(tex_coords);

    vec3 N = vertex_normal;
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangent_normal);
}
