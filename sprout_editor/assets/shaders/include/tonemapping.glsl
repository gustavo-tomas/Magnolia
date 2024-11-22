// Some tonemapping functions
// Relevant links:
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/includes/tonemapping.glsl
// http://filmicworlds.com/blog/filmic-tonemapping-operators/

vec3 gamma_correction(vec3 color)
{
    vec3 corrected_color = pow(color, vec3(0.454545)); // 0.4545... == vec3(1.0 / 2.2)
    return corrected_color;
}

vec3 tonemapping_hdr(vec3 color)
{
    vec3 corrected_color = color / (color + vec3(1.0));
    return corrected_color;
}

vec3 tonemapping_reinhard(vec3 color)
{
    // color *= 16;  // Hardcoded Exposure Adjustment
    vec3 x = max(vec3(0), color - 0.004);
    vec3 corrected_color = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
    return corrected_color;
}

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 uncharted2_tonemap(vec3 x)
{
   return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 tonemapping_uncharted2(vec3 color)
{
    // color *= 16;  // Hardcoded Exposure Adjustment

    float exposure_bias = 2.0;
    vec3 curr = uncharted2_tonemap(exposure_bias * color);

    vec3 white_scale = 1.0 / uncharted2_tonemap(vec3(W));
    vec3 adjusted_color = curr * white_scale;
        
    vec3 corrected_color = pow(adjusted_color, vec3(1 / 2.2));
    return corrected_color;
}
