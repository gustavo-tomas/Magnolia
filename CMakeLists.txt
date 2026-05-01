# @TODO: finish debug configuration
# @TODO: update python script
# @TODO: update readme
# @TODO: release configuration
# @TODO: dist configuration
# @TODO: windows version

cmake_minimum_required(VERSION 4.3.2)

################################################################################
# Engine
################################################################################

set (CMAKE_CXX_STANDARD 23)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(magnolia)

# Project variables

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${SYSTEM_NAME}/${BUILD_TYPE}/bin)       # .exe, .dll
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${SYSTEM_NAME}/${BUILD_TYPE}/bin/lib)   # .so
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${SYSTEM_NAME}/${BUILD_TYPE}/bin/lib)   # .a, .lib

# Copy vulkan libs

file(GLOB PREBUILT_LIBS "${CMAKE_SOURCE_DIR}/ext/${SYSTEM_NAME}/*.so*")

file(COPY
    ${PREBUILT_LIBS}
    DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

set(MAG_SOURCES
    "${PROJECT_NAME}/src/platform/backend/sdl/conversions.hpp"
    "${PROJECT_NAME}/src/platform/backend/sdl/window.cpp"
    "${PROJECT_NAME}/src/platform/platform.cpp"
    "${PROJECT_NAME}/src/platform/file_system.cpp"
    "${PROJECT_NAME}/src/threads/process_manager.cpp"
    "${PROJECT_NAME}/src/threads/job_system.cpp"
    "${PROJECT_NAME}/src/threads/thread.cpp"
    "${PROJECT_NAME}/src/scripting/scripting_engine.cpp"
    "${PROJECT_NAME}/src/project/project.cpp"
    "${PROJECT_NAME}/src/core/engine.cpp"
    "${PROJECT_NAME}/src/core/memory.cpp"
    "${PROJECT_NAME}/src/core/buffer.cpp"
    "${PROJECT_NAME}/src/core/event.cpp"
    "${PROJECT_NAME}/src/core/string.cpp"
    "${PROJECT_NAME}/src/tools/model_importer.cpp"
    "${PROJECT_NAME}/src/tools/console.cpp"
    "${PROJECT_NAME}/src/tools/profiler.cpp"
    "${PROJECT_NAME}/src/pch.hpp"
    "${PROJECT_NAME}/src/math/types.cpp"
    "${PROJECT_NAME}/src/gfx/gfx.cpp"
    "${PROJECT_NAME}/src/gfx/backend/backend.hpp"
    "${PROJECT_NAME}/src/gfx/backend/vulkan/conversions.hpp"
    "${PROJECT_NAME}/src/gfx/backend/vulkan/vulkan.cpp"
    "${PROJECT_NAME}/src/physics/backend/jolt/conversions.hpp"
    "${PROJECT_NAME}/src/physics/backend/jolt/physics.cpp"
    "${PROJECT_NAME}/src/physics/backend/jolt/implementations.hpp"
    "${PROJECT_NAME}/src/audio/audio_system.cpp"
    "${PROJECT_NAME}/src/camera/frustum.cpp"
    "${PROJECT_NAME}/src/camera/camera.cpp"
    "${PROJECT_NAME}/src/resources/shader.cpp"
    "${PROJECT_NAME}/src/resources/audio.cpp"
    "${PROJECT_NAME}/src/resources/material.cpp"
    "${PROJECT_NAME}/src/resources/resource.cpp"
    "${PROJECT_NAME}/src/resources/model.cpp"
    "${PROJECT_NAME}/src/resources/font.cpp"
    "${PROJECT_NAME}/src/resources/texture.cpp"
    "${PROJECT_NAME}/include/magnolia/platform/file_system.hpp"
    "${PROJECT_NAME}/include/magnolia/platform/json.hpp"
    "${PROJECT_NAME}/include/magnolia/platform/platform.hpp"
    "${PROJECT_NAME}/include/magnolia/platform/serializer.hpp"
    "${PROJECT_NAME}/include/magnolia/platform/window.hpp"
    "${PROJECT_NAME}/include/magnolia/threads/containers.hpp"
    "${PROJECT_NAME}/include/magnolia/threads/job_system.hpp"
    "${PROJECT_NAME}/include/magnolia/threads/process_manager.hpp"
    "${PROJECT_NAME}/include/magnolia/threads/thread.hpp"
    "${PROJECT_NAME}/include/magnolia/scripting/scripting_engine.hpp"
    "${PROJECT_NAME}/include/magnolia/project/project.hpp"
    "${PROJECT_NAME}/include/magnolia/core/keys.hpp"
    "${PROJECT_NAME}/include/magnolia/core/engine.hpp"
    "${PROJECT_NAME}/include/magnolia/core/buffer.hpp"
    "${PROJECT_NAME}/include/magnolia/core/memory.hpp"
    "${PROJECT_NAME}/include/magnolia/core/string.hpp"
    "${PROJECT_NAME}/include/magnolia/core/assert.hpp"
    "${PROJECT_NAME}/include/magnolia/core/debug.hpp"
    "${PROJECT_NAME}/include/magnolia/core/types.hpp"
    "${PROJECT_NAME}/include/magnolia/core/logger.hpp"
    "${PROJECT_NAME}/include/magnolia/core/event.hpp"
    "${PROJECT_NAME}/include/magnolia/tools/profiler.hpp"
    "${PROJECT_NAME}/include/magnolia/tools/model_importer.hpp"
    "${PROJECT_NAME}/include/magnolia/tools/console.hpp"
    "${PROJECT_NAME}/include/magnolia/math/types.hpp"
    "${PROJECT_NAME}/include/magnolia/gfx/gfx.hpp"
    "${PROJECT_NAME}/include/magnolia/gfx/types.hpp"
    "${PROJECT_NAME}/include/magnolia/physics/physics.hpp"
    "${PROJECT_NAME}/include/magnolia/audio/audio_system.hpp"
    "${PROJECT_NAME}/include/magnolia/ecs/ecs.hpp"
    "${PROJECT_NAME}/include/magnolia/camera/camera.hpp"
    "${PROJECT_NAME}/include/magnolia/camera/frustum.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/resource.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/material.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/audio.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/shader.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/texture.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/font.hpp"
    "${PROJECT_NAME}/include/magnolia/resources/model.hpp"
)

add_library(${PROJECT_NAME} SHARED ${MAG_SOURCES})

set(MAG_LIB_INCLUDES
    "${CMAKE_SOURCE_DIR}/libs"
    "${CMAKE_SOURCE_DIR}/libs/assimp/include"
    "${CMAKE_SOURCE_DIR}/libs/freetype/include"
    "${CMAKE_SOURCE_DIR}/libs/glm"
    "${CMAKE_SOURCE_DIR}/libs/imgui"
    "${CMAKE_SOURCE_DIR}/libs/jolt"
    "${CMAKE_SOURCE_DIR}/libs/json/single_include"
    "${CMAKE_SOURCE_DIR}/libs/meshoptimizer/src"
    "${CMAKE_SOURCE_DIR}/libs/sdl/include"
    "${CMAKE_SOURCE_DIR}/libs/soloud/include"
    "${CMAKE_SOURCE_DIR}/libs/spirv_reflect"
    "${CMAKE_SOURCE_DIR}/libs/stb"
    "${CMAKE_SOURCE_DIR}/libs/vkbootstrap/src"
    "${CMAKE_SOURCE_DIR}/libs/vma/include"
    "${CMAKE_SOURCE_DIR}/libs/vulkan/include"
)

add_compile_definitions(
    "GLM_FORCE_QUAT_DATA_WXYZ"
    "GLM_ENABLE_EXPERIMENTAL"
    "JPH_PROFILE_ENABLED"
    "JPH_DEBUG_RENDERER"
    "JPH_OBJECT_STREAM"
    "MAG_BUILD_SHARED"
    "MAG_CONFIG_DEBUG"
    "MAG_ASSERTIONS_ENABLED"
    "MAG_PROFILE_ENABLED"
    "VULKAN_HPP_NAMESPACE=vk"
    "VULKAN_HPP_NO_EXCEPTIONS"
)

# path to include
target_include_directories(${PROJECT_NAME}
    PRIVATE "${PROJECT_NAME}/src"
    PRIVATE "${PROJECT_NAME}/include"
    PRIVATE "${MAG_LIB_INCLUDES}"
)

# pch
target_precompile_headers(${PROJECT_NAME}
    PRIVATE "${PROJECT_NAME}/src/pch.hpp"
)

# path to lib
target_link_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

add_subdirectory("${CMAKE_SOURCE_DIR}/scripts/imgui" "imgui")

add_subdirectory("${CMAKE_SOURCE_DIR}/scripts/soloud" "soloud")

add_subdirectory("${CMAKE_SOURCE_DIR}/libs/meshoptimizer" "meshoptimizer")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL OFF CACHE BOOL "")
add_subdirectory("${CMAKE_SOURCE_DIR}/libs/assimp" "assimp")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(SKIP_INSTALL_ALL ON CACHE BOOL "") 
set(FT_DISABLE_ZLIB ON CACHE BOOL "") 
set(FT_DISABLE_BZIP2 ON CACHE BOOL "") 
set(FT_DISABLE_PNG ON CACHE BOOL "")
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "") 
set(FT_DISABLE_BROTLI ON CACHE BOOL "") 
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "")
add_subdirectory("${CMAKE_SOURCE_DIR}/libs/freetype" "freetype")

set(VK_BOOTSTRAP_POSITION_INDEPENDENT_CODE ON CACHE BOOL "")
set(VK_BOOTSTRAP_WERROR ON CACHE BOOL "")
set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "")
set(VK_BOOTSTRAP_VULKAN_HEADER_DIR "${CMAKE_SOURCE_DIR}/libs/vulkan/include" CACHE BOOL "")
add_subdirectory("${CMAKE_SOURCE_DIR}/libs/vkbootstrap" "vk-bootstrap")

set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "")
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "")
set(SPIRV_REFLECT_BUILD_TESTS OFF CACHE BOOL "")
set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "")
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "")
add_subdirectory("${CMAKE_SOURCE_DIR}/libs/spirv_reflect" "spirv")

set(CPP_RTTI_ENABLED ON CACHE BOOL "")
set(CMAKE_CXX_COMPILER "clang++" CACHE BOOL "")
set(ENABLE_INSTALL OFF CACHE BOOL "")
set(TARGET_UNIT_TESTS OFF CACHE BOOL "")
set(TARGET_HELLO_WORLD OFF CACHE BOOL "")
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "")
set(TARGET_SAMPLES OFF CACHE BOOL "")
set(TARGET_VIEWER OFF CACHE BOOL "")
set(FLOATING_POINT_EXCEPTIONS_ENABLED OFF CACHE BOOL "")
    
# Use CPU compute
set(JPH_USE_DX12 OFF CACHE BOOL "")
set(JPH_USE_DXC OFF CACHE BOOL "")
set(JPH_USE_MTL OFF CACHE BOOL "")
set(JPH_USE_VK OFF CACHE BOOL "")
set(JPH_USE_CPU_COMPUTE ON CACHE BOOL "")

# Only enables widely available instruction sets
set(USE_SSE4_1 ON CACHE BOOL "")
set(USE_SSE4_2 ON CACHE BOOL "")
set(USE_AVX OFF CACHE BOOL "")
set(USE_AVX2 OFF CACHE BOOL "")
set(USE_AVX512 OFF CACHE BOOL "")
set(USE_LZCNT OFF CACHE BOOL "")
set(USE_TZCNT OFF CACHE BOOL "")
set(USE_F16C OFF CACHE BOOL "")
set(USE_FMADD OFF CACHE BOOL "")
add_subdirectory("${CMAKE_SOURCE_DIR}/libs/jolt/Build" "jolt")

# links
target_link_libraries(${PROJECT_NAME}
    PRIVATE vulkan
    PRIVATE vk-bootstrap
    PRIVATE spirv-reflect-static
    PRIVATE SDL3
    PRIVATE assimp
    PRIVATE meshoptimizer
    PRIVATE freetype
    PRIVATE soloud
    PRIVATE imgui
    PRIVATE Jolt
)

################################################################################
# Client Application
################################################################################

project(test_game)

set(TEST_GAME_SOURCES
    "${PROJECT_NAME}/src/scene.hpp"
    "${PROJECT_NAME}/src/renderer.cpp"
    "${PROJECT_NAME}/src/application.hpp"
    "${PROJECT_NAME}/src/scene_serializer.hpp"
    "${PROJECT_NAME}/src/scene.cpp"
    "${PROJECT_NAME}/src/entry_point.cpp"
    "${PROJECT_NAME}/src/scene_serializer.cpp"
    "${PROJECT_NAME}/src/renderer.hpp"
    "${PROJECT_NAME}/src/application.cpp"
    "${PROJECT_NAME}/src/ecs/systems.cpp"
    "${PROJECT_NAME}/src/ecs/debug.hpp"
    "${PROJECT_NAME}/src/ecs/components.hpp"
    "${PROJECT_NAME}/src/ecs/debug.cpp"
    "${PROJECT_NAME}/src/ecs/systems.hpp"
)

add_executable(${PROJECT_NAME} ${TEST_GAME_SOURCES})

add_compile_definitions(
    "GLM_FORCE_QUAT_DATA_WXYZ"
    "GLM_ENABLE_EXPERIMENTAL"
    "MAG_CONFIG_DEBUG"
    "MAG_ASSERTIONS_ENABLED"
    "MAG_PROFILE_ENABLED"
)

# path to include
target_include_directories(${PROJECT_NAME}
    PRIVATE "${PROJECT_NAME}/src"
    PRIVATE "${CMAKE_SOURCE_DIR}/magnolia/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/libs/json/single_include"
    PRIVATE "${CMAKE_SOURCE_DIR}/libs/glm"
)

# path to lib
target_link_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

# links
target_link_libraries(${PROJECT_NAME}
    PRIVATE magnolia
)
