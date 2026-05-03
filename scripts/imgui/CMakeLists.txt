cmake_minimum_required(VERSION 4.3.2)

project(imgui)

set(IMGUI_SOURCES
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imconfig.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui_internal.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imstb_rectpack.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imstb_textedit.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imstb_truetype.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/misc/cpp/imgui_stdlib.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/misc/cpp/imgui_stdlib.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui_draw.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui_tables.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui_widgets.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/imgui_demo.cpp"

    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdl3.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdl3.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdlrenderer3.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdlrenderer3.cpp"
)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE
    # ALL configurations
    # -Wall -Wextra
    
    # Debug
    $<$<CONFIG:debug>:-DDEBUG -O0 -g>

    # Release
    $<$<CONFIG:release>:-DNDEBUG -O2>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}"
    "${MAG_ROOT_DIR}/libs/sdl/include"
)
