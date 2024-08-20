// See this: https://developer.nvidia.com/vulkan-shader-resource-binding

// Macros
#define MODEL_MATRIX u_instance.models[gl_InstanceIndex].model
#define PROJ_MATRIX u_global.projection
#define VIEW_MATRIX u_global.view

#define ALBEDO_TEXTURE u_material_textures[0]
#define NORMAL_TEXTURE u_material_textures[1]

// Constants
const uint MAX_NUMBER_OF_LIGHTS = 4;

// Types
struct Light
{
	vec3 color;
	float intensity;
	vec3 position;
};

struct Model
{
	mat4 model;
};

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 near_far;

    // Lights
    Light point_lights[MAX_NUMBER_OF_LIGHTS];
} u_global;

// Instance buffer
layout (std140, set = 1, binding = 0) readonly buffer InstanceBuffer
{
    Model models[];
} u_instance;

// Shader parameters
layout (set = 2, binding = 0) uniform ShaderBuffer
{
    // Debug
    uint texture_output;
    uint normal_output;
} u_shader;

// Materials
// 0 - Albedo | 1 - Normal
layout (set = 3, binding = 0) uniform sampler2D u_material_textures[2];
