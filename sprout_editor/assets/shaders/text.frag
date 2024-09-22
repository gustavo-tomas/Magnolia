#version 460

#include "text.include.glsl"

layout (location = 0) in vec2 in_tex_coords;

layout (location = 0) out vec4 out_frag_color;

// @TODO: move this to utils or smth
float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screen_pixel_range(float pixel_range, sampler2D tex, vec2 tex_coord)
{
    vec2 unit_range = vec2(pixel_range) / vec2(textureSize(tex, 0));
    vec2 screen_tex_size = vec2(1.0) / fwidth(tex_coord);
    return max(0.5 * dot(unit_range, screen_tex_size), 1.0);
}

void main()
{
    vec4 msd = texture(u_atlas_texture, in_tex_coords).rgba;
    float sd = median(msd.r, msd.g, msd.b);
    
    float screen_pixel_distance = 
        screen_pixel_range(u_text_info.pixel_range, u_atlas_texture, in_tex_coords) * (sd - 0.5);

    float opacity = clamp(screen_pixel_distance + 0.5, 0.0, 1.0);

    // @TODO: the 'perfect' blending needs to mix between the background and text color. To achieve that, we can sample
    // the color from a render attachment and use it as background color. However, to do that, we would need to handle
    // descriptors and im not going to do that rn.
    // out_frag_color = mix(u_text_info.color, background_color, opacity);

    if (opacity < 0.5) discard; // Discard transparent fragments

    out_frag_color = vec4(u_text_info.color.rgb, opacity);
}
