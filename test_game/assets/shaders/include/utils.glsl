// Conversions, common calculations, etc

#define PI      3.1415926535
#define HALF_PI 1.5707963268

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

// glsl has no rotation functions so we need assistance
// https://gist.github.com/yiwenl/3f804e80d0930e34a0b33359259b556c
mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec3 rotate(vec3 v, vec3 axis, float angle) {
	mat4 m = rotationMatrix(axis, angle);
	return (m * vec4(v, 1.0)).xyz;
}

vec4 mod289(vec4 x)
{ 
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
    return mod289(((x * 34.0) + 10.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Classic Perlin noise
// https://github.com/stegu/webgl-noise
float perlin_noise(vec2 p)
{
    vec4 pi = floor(p.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 pf = fract(p.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    pi = mod289(pi); // To avoid truncation effects in permutation
    
    vec4 ix = pi.xzxz;
    vec4 iy = pi.yyww;
    vec4 fx = pf.xzxz;
    vec4 fy = pf.yyww;

    vec4 i = permute(permute(ix) + iy);

    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;

    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);

    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;  
    g01 *= norm.y;  
    g10 *= norm.z;  
    g11 *= norm.w;  

    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));

    vec2 fade_xy = fade(pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    
    return 2.3 * n_xy;
}

float perlin_noise_normalized(vec2 p)
{
    return (perlin_noise(p) * 0.5) + 0.5;
}

#ifdef FRAGMENT_SHADER

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

#endif
