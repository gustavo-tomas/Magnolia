#include "include/common.glsl"

#define ALBEDO_TEXTURE u_material_textures[0]
#define NORMAL_TEXTURE u_material_textures[1]
#define ROUGHNESS_TEXTURE u_material_textures[2]
#define METALNESS_TEXTURE u_material_textures[3]

// Types
struct Model
{
	mat4 model;
};

struct Material 
{
	vec4 base_color;
    float roughness;
    float metallic;
};

struct Light
{
	vec3 color;
	float intensity;
	vec3 position;
};

// @TODO: for now, only fragment shaders support push constants
// Push constants (dont exceed 128 bytes)
layout (push_constant) uniform PushConstants
{
    // Shader debug parameters
    uint texture_output;
    uint normal_output;

    // Scene parameters
    uint number_of_lights;

    // Material parameters
    uint material_index;
} u_push_constants;

// Global buffer
layout (set = 0, binding = 0) uniform GlobalBuffer
{
    // Camera
    mat4 view;
    mat4 projection;
    vec2 near_far;
} u_global;

// Lights buffer
layout (std140, set = 1, binding = 0) readonly buffer LightBuffer
{
    Light lights[];
} u_lights;

// Instance buffer
layout (std140, set = 2, binding = 0) readonly buffer InstanceBuffer
{
    Model models[];
} u_instance;

// Materials buffer
layout (std140, set = 3, binding = 0) readonly buffer MaterialBuffer
{
    Material materials[];
} u_material;

// Material textures
// 0 - Albedo | 1 - Normal
layout (set = 4, binding = 0) uniform sampler2D u_material_textures[4];
