// See this: https://developer.nvidia.com/vulkan-shader-resource-binding

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    mat4 view;
    mat4 projection;
    vec2 near_far;
} u_global;

// Instance buffer
layout (set = 1, binding = 0) uniform InstanceBuffer
{
    mat4 model;
} u_instance;

// Materials 
layout (set = 2, binding = 0) uniform sampler2D u_diffuse_texture;
layout (set = 2, binding = 1) uniform sampler2D u_normal_texture;

// Light buffer
layout (set = 3, binding = 0) uniform LightBuffer
{
    vec4 color_intensity;
    vec3 position;
} u_light;
