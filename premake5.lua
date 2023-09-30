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
    kind "ConsoleApp"

    targetdir ("bin/%{cfg.system}/%{cfg.buildcfg}")
    objdir ("bin/%{cfg.system}/build/%{cfg.buildcfg}")

    files
    {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs 
    { 

    }

    links
    {

    }

    filter "system:linux"
        pic "On"

    filter "configurations:debug"
        buildoptions { "-Wall", "-Wextra" }
        defines { "MAG_DEBUG" }
        symbols "On" -- '-g'
        optimize "Off" -- '-O0'
        runtime "debug"

    filter "configurations:profile"
        defines { "MAG_PROFILE" }
        symbols "Off"
        optimize "On" -- '-O2'
        runtime "release"

    filter "configurations:release"
        defines { "MAG_RELEASE" }
        symbols "Off"
        optimize "Full" -- '-O3'
        runtime "release"
