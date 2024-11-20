// See this: https://developer.nvidia.com/vulkan-shader-resource-binding

// Macros
#define MODEL_MATRIX u_instance.models[gl_InstanceIndex].model
#define PROJ_MATRIX u_global.projection
#define VIEW_MATRIX u_global.view
#define NEAR_FAR u_global.near_far

// Constants
const float PI = 3.1415926535;
