cmake_minimum_required(VERSION 4.3.2)

project(imgui)

set(IMGUI_SOURCES
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imconfig.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui_internal.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imstb_rectpack.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imstb_textedit.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imstb_truetype.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/misc/cpp/imgui_stdlib.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/misc/cpp/imgui_stdlib.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/imgui_demo.cpp"

    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdl3.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdl3.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdlrenderer3.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/backends/imgui_impl_sdlrenderer3.cpp"
)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}"
    PRIVATE "${CMAKE_SOURCE_DIR}/libs/sdl/include"
)
