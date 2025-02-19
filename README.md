# Magnolia

![screenshot](sprout_editor/assets/screenshots/magnolia_v0.6.0.png)
> Current state of the engine

> Windows build is still in development (for now is linux only)!

## Requirements
<!-- Add links -->
- C++20
- Gcc 11.4.0
- Clang 15
- Python 3.10.12
- Cmake 3.22.1
- Vulkan SDK 1.3.268
- [Mold linker](https://github.com/rui314/mold) (Optional, can be removed/swapped by changing the `linkoptions` field in the [`premake5.lua`](premake5.lua) file)

## Build
To build and run the program use

```
python3 build.py debug
```

## References
- [[VulkanAbstractionLayer](https://github.com/asc-community/VulkanAbstractionLayer)] Renderer architecture and core structures
- [[Godot](https://github.com/godotengine/godot)] Lib management
- [[VulkanTutorial](https://github.com/Overv/VulkanTutorial)] Vulkan foundations
- [[VkGuide](https://github.com/vblanco20-1/vulkan-guide)] Render to texture and other vulkan optimizations
- [[ASliceOfRendering](https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/)] Generating an infinite grid using shaders
- [[Hazel](https://github.com/TheCherno/Hazel)] Content browser and other editor widgets
- [[FontAwesome](https://github.com/FortAwesome/Font-Awesome/)] Icon fonts
- [[IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)] Font awesome for C++
- [[OGLDEV](https://www.youtube.com/watch?v=9HO1dl0zcxg)] Mesh optimizations using meshoptimizer
- [[LearnOpenGL](https://learnopengl.com/Lighting/Basic-Lighting)] Lighting and basic computer graphics concepts
- [[WickedEngine](https://wickedengine.net/)] ECS and other graphics systems
- [[Kohi](https://github.com/travisvroman/kohi)] Rendering systems, native data file formats
- [[Scion2D](https://github.com/dwjclark11/Scion2D)] Lua scripting
- [[podgorskiy](https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644)] Frustum culling
- [[Inigo Quilez](https://iquilezles.org/articles/frustumcorrect/)] Frustum culling algorithm
- [[OpenGLTutorial](https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/)] Billboarding
- [[Vulkan-glTF-PBR](https://github.com/SaschaWillems/Vulkan-glTF-PBR)] PBR and tonemapping algorithms
- [[Filmic Worlds](http://filmicworlds.com/blog/filmic-tonemapping-operators/)] Tonemapping algorithms
- [[glTF-Sample-Models](https://github.com/KhronosGroup/glTF-Sample-Models)] Better quality test models
