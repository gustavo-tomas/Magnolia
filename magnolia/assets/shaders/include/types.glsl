// See this: https://developer.nvidia.com/vulkan-shader-resource-binding

// Constants
const int MAX_NUMBER_OF_LIGHTS = 4;

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 near_far;

    // Lights
    vec4 point_light_color_and_intensity[MAX_NUMBER_OF_LIGHTS];
    vec4 point_light_position[MAX_NUMBER_OF_LIGHTS];
} u_global;

// Instance buffer
layout (set = 1, binding = 0) uniform InstanceBuffer
{
    mat4 model;
} u_instance;

// Materials 
layout (set = 2, binding = 0) uniform sampler2D u_diffuse_texture;
layout (set = 2, binding = 1) uniform sampler2D u_normal_texture;
