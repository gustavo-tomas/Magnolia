-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "magnolia"
    architecture "x86_64"
    toolset "clang"
    language "c++"
    cppdialect "c++20"
    configurations { "debug", "profile", "release" }
    location "build"

    -- Output directories
    targetdir ("build/%{cfg.system}/%{cfg.buildcfg}")
    objdir ("build/%{cfg.system}/obj/%{cfg.buildcfg}/%{prj.name}")

-- Engine --------------------------------------------------------------------------------------------------------------
project "magnolia"
    kind "consoleapp"

    files
    {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs 
    { 
        "%{prj.name}/src",
        "libs/sdl/include",
        "libs/fmt/include",
        "libs/glm"
    }

    -- @TODO: finish lib generation
    links
    {
        "SDL2", "vulkan", "fmt"
    }

    filter "system:linux"
        pic "on"

    filter "configurations:debug"
        buildoptions { "-Wall", "-Wextra" }
        defines { "MAG_DEBUG" }
        symbols "on" -- '-g'
        optimize "off" -- '-O0'
        runtime "debug"

    filter "configurations:profile"
        defines { "MAG_PROFILE" }
        symbols "off"
        optimize "on" -- '-O2'
        runtime "release"

    filter "configurations:release"
        defines { "MAG_RELEASE" }
        symbols "off"
        optimize "full" -- '-O3'
        runtime "release"


-- Libs ----------------------------------------------------------------------------------------------------------------

-- fmt -----------------------------------------------------------------------------------------------------------------
project "fmt"
    kind "staticlib"

    files
    {
        "libs/fmt/include/fmt/**.h",
        "libs/fmt/src/**.cc"
    }

    includedirs 
    { 
        "libs/fmt/include"
    }

    filter { "files:**.cc" }
        compileas "module"
