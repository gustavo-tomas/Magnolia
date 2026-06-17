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

set(MAG_ROOT_DIR "${CMAKE_SOURCE_DIR}/..")
set(MAG_BIN_DIR "${CMAKE_BINARY_DIR}/../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MAG_BIN_DIR})       # .exe, .dll
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MAG_BIN_DIR}/lib)   # .so
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MAG_BIN_DIR}/lib)   # .a, .lib

# Copy vulkan libs

file(GLOB PREBUILT_LIBS "${MAG_ROOT_DIR}/ext/${SYSTEM_NAME}/*.so*")

file(COPY
    ${PREBUILT_LIBS}
    DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

set(MAG_SOURCES
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/platform/backend/sdl/conversions.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/platform/backend/sdl/window.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/platform/platform.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/platform/file_system.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/threads/process_manager.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/threads/job_system.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/threads/thread.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/scripting/scripting_engine.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/project/project.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/core/engine.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/core/memory.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/core/buffer.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/core/event.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/core/string.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/tools/model_importer.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/tools/console.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/tools/profiler.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/pch.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/math/types.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/math/functions.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/gfx/gfx.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/gfx/backend/backend.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/gfx/backend/vulkan/conversions.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/gfx/backend/vulkan/vulkan.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/physics/backend/jolt/conversions.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/physics/backend/jolt/physics.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/physics/backend/jolt/implementations.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/audio/audio_system.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/camera/frustum.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/camera/camera.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/shader.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/audio.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/material.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/resource.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/model.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/font.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/resources/texture.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/platform/file_system.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/platform/json.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/platform/platform.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/platform/serializer.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/platform/window.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/threads/containers.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/threads/job_system.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/threads/process_manager.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/threads/thread.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/scripting/scripting_engine.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/project/project.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/keys.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/engine.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/buffer.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/memory.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/string.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/assert.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/debug.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/types.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/logger.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/core/event.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/tools/profiler.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/tools/model_importer.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/tools/console.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/math/types.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/math/functions.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/gfx/gfx.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/gfx/types.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/physics/physics.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/audio/audio_system.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/ecs/ecs.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/camera/camera.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/camera/frustum.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/resource.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/material.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/audio.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/shader.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/texture.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/font.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include/magnolia/resources/model.hpp"
)

add_library(${PROJECT_NAME} SHARED ${MAG_SOURCES})

set(MAG_LIB_INCLUDES
    "${MAG_ROOT_DIR}/libs"
    "${MAG_ROOT_DIR}/libs/assimp/include"
    "${MAG_ROOT_DIR}/libs/freetype/include"
    "${MAG_ROOT_DIR}/libs/imgui"
    "${MAG_ROOT_DIR}/libs/jolt"
    "${MAG_ROOT_DIR}/libs/json/single_include"
    "${MAG_ROOT_DIR}/libs/meshoptimizer/src"
    "${MAG_ROOT_DIR}/libs/sdl/include"
    "${MAG_ROOT_DIR}/libs/soloud/include"
    "${MAG_ROOT_DIR}/libs/spirv_reflect"
    "${MAG_ROOT_DIR}/libs/stb"
    "${MAG_ROOT_DIR}/libs/vkbootstrap/src"
    "${MAG_ROOT_DIR}/libs/vma/include"
    "${MAG_ROOT_DIR}/libs/vulkan/include"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # All configurations
    JPH_PROFILE_ENABLED
    JPH_DEBUG_RENDERER
    JPH_OBJECT_STREAM
    MAG_BUILD_SHARED
    MAG_PROFILE_ENABLED
    VULKAN_HPP_NAMESPACE=vk
    VULKAN_HPP_NO_EXCEPTIONS
    
    # Debug
    $<$<CONFIG:debug>:MAG_CONFIG_DEBUG MAG_ASSERTIONS_ENABLED>

    # Release
    $<$<CONFIG:release>:MAG_CONFIG_RELEASE>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    # ALL configurations
    -Wall -Wextra -fno-exceptions -fvisibility=hidden -ftime-trace
    
    # Debug
    $<$<CONFIG:debug>:-DDEBUG -O0 -g>

    # Release
    $<$<CONFIG:release>:-DNDEBUG -O2>
)

# path to include
target_include_directories(${PROJECT_NAME} PRIVATE
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/include"
    "${MAG_LIB_INCLUDES}"
)

# pch
target_precompile_headers(${PROJECT_NAME} PRIVATE
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/pch.hpp"
)

# path to lib
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

# Libs

add_subdirectory("${MAG_ROOT_DIR}/scripts/imgui" "imgui")

add_subdirectory("${MAG_ROOT_DIR}/scripts/soloud" "soloud")

add_subdirectory("${MAG_ROOT_DIR}/libs/meshoptimizer" "meshoptimizer")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL OFF CACHE BOOL "")
set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "")
add_subdirectory("${MAG_ROOT_DIR}/libs/assimp" "assimp")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(SKIP_INSTALL_ALL ON CACHE BOOL "") 
set(FT_DISABLE_ZLIB ON CACHE BOOL "") 
set(FT_DISABLE_BZIP2 ON CACHE BOOL "") 
set(FT_DISABLE_PNG ON CACHE BOOL "")
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "") 
set(FT_DISABLE_BROTLI ON CACHE BOOL "") 
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "")
add_subdirectory("${MAG_ROOT_DIR}/libs/freetype" "freetype")

set(VK_BOOTSTRAP_POSITION_INDEPENDENT_CODE ON CACHE BOOL "")
set(VK_BOOTSTRAP_WERROR ON CACHE BOOL "")
set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "")
set(VK_BOOTSTRAP_VULKAN_HEADER_DIR "${MAG_ROOT_DIR}/libs/vulkan/include" CACHE BOOL "")
add_subdirectory("${MAG_ROOT_DIR}/libs/vkbootstrap" "vk-bootstrap")

set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "")
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "")
set(SPIRV_REFLECT_BUILD_TESTS OFF CACHE BOOL "")
set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "")
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "")
add_subdirectory("${MAG_ROOT_DIR}/libs/spirv_reflect" "spirv-reflect")

set(SDL_SHARED OFF CACHE BOOL "") 
set(SDL_STATIC ON CACHE BOOL "") 
set(SDL_EXAMPLES OFF CACHE BOOL "") 
set(SDL_TESTS OFF CACHE BOOL "") 
set(SDL_TEST_LIBRARY OFF CACHE BOOL "")
set(SDL_INSTALL OFF CACHE BOOL "") 
set(SDL_DISABLE_INSTALL_DOCS OFF CACHE BOOL "") 
set(SDL_INSTALL_TESTS OFF CACHE BOOL "")
add_subdirectory("${MAG_ROOT_DIR}/libs/sdl" "sdl")

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
add_subdirectory("${MAG_ROOT_DIR}/libs/jolt/Build" "jolt")

# links
target_link_libraries(${PROJECT_NAME} PRIVATE
    vulkan
    vk-bootstrap
    spirv-reflect-static
    SDL3
    assimp
    meshoptimizer
    freetype
    soloud
    imgui
    Jolt
)

################################################################################
# Client Application
################################################################################

project(test_game)

set(TEST_GAME_SOURCES
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/scene.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/renderer.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/application.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/scene_serializer.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/scene.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/entry_point.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/scene_serializer.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/renderer.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/application.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/ecs/systems.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/ecs/debug.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/ecs/components.hpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/ecs/debug.cpp"
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src/ecs/systems.hpp"
)

add_executable(${PROJECT_NAME} ${TEST_GAME_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # All configurations
    MAG_PROFILE_ENABLED
    
    # Debug
    $<$<CONFIG:debug>:MAG_CONFIG_DEBUG MAG_ASSERTIONS_ENABLED>

    # Release
    $<$<CONFIG:release>:MAG_CONFIG_RELEASE>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    # ALL configurations
    -Wall -Wextra -fno-exceptions -ftime-trace
    
    # Debug
    $<$<CONFIG:debug>:-DDEBUG -O0 -g>

    # Release
    $<$<CONFIG:release>:-DNDEBUG -O2>
)

# path to include
target_include_directories(${PROJECT_NAME} PRIVATE
    "${MAG_ROOT_DIR}/${PROJECT_NAME}/src"
    "${MAG_ROOT_DIR}/magnolia/include"
    "${MAG_ROOT_DIR}/libs/json/single_include"
)

# path to lib
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

# links
target_link_libraries(${PROJECT_NAME} PRIVATE
    magnolia
)
