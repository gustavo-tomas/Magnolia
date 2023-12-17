-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "magnolia"
    architecture "x86_64"
    toolset "clang"
    language "c++"
    cppdialect "c++20"
    configurations { "debug", "profile", "release" }

-- Engine --------------------------------------------------------------------------------------------------------------
project "magnolia"
    location "magnolia"
    kind "consoleapp"

    targetdir ("bin/%{cfg.system}/%{cfg.buildcfg}")
    objdir ("bin/%{cfg.system}/build/%{cfg.buildcfg}")

    files
    {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs 
    { 
        "%{prj.name}/src"
    }

    links
    {
        "SDL2"
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
