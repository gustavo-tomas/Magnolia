// See this: https://developer.nvidia.com/vulkan-shader-resource-binding

// Macros
#define MODEL_MATRIX u_instance.models[gl_InstanceIndex].model
#define PROJ_MATRIX u_global.projection
#define VIEW_MATRIX u_global.view
#define NEAR_FAR u_global.near_far

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
