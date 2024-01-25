-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "magnolia"
    architecture "x86_64"
    language "c++"
    cppdialect "c++20"
    configurations { "debug", "profile", "release" }
    location "build"
    staticruntime "On"
    
    -- Output directories
    targetdir ("build/%{cfg.system}")
    objdir ("build/%{cfg.system}/obj/%{cfg.buildcfg}/%{prj.name}")
    
    filter "system:linux"
        toolset "clang"
        
    filter "system:windows"
        toolset "gcc"
        
        -- Engine --------------------------------------------------------------------------------------------------------------
project "magnolia"
    targetname ("%{prj.name}_%{cfg.buildcfg}")
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
        "libs/vulkan/include",
        "libs/vma/include",
        "libs/glm"
    }

    links
    {
        "fmt"--, "sdl"
    }

    filter "system:linux"
        pic "on"
        links
        {
            "vulkan", "sdl"
        }

    filter "system:windows"
        systemversion "latest"

        libdirs
        { 
            "build/%{cfg.system}/lib",
        }

        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        links
        {
            "vulkan-1",
            "SDL2",
            "SDL2main",
        }
        -- entrypoint("mainCRTStartup")            
        
    filter "configurations:debug"
        buildoptions { "-Wall", "-Wextra" }
        defines { "MAG_DEBUG", "MAG_ASSERTIONS_ENABLED" }
        symbols "on" -- '-g'
        optimize "off" -- '-O0'
        runtime "debug"

    filter "configurations:profile"
        defines { "MAG_PROFILE" }
        symbols "off"
        optimize "on" -- '-O2'
        runtime "release"

    filter "configurations:release"
        defines { "MAG_RELEASE", "MAG_ASSERTIONS_ENABLED" } -- @TODO: fix this
        symbols "off"
        optimize "full" -- '-O3'
        runtime "release"


-- Libs ----------------------------------------------------------------------------------------------------------------

-- fmt -----------------------------------------------------------------------------------------------------------------
project "fmt"
    filter "system:linux"
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
        -- buildoptions
        -- {
        --     "-fmodules-ts"
        -- }
        
    filter "system:windows"
        kind "None"
        targetextension ".a"
        targetprefix "lib"
        
        function exists(filePath)
            local file = io.open(filePath, "r")
            if file then
                file:close()
                return true
            else
                return false
            end
        end

        if exists("build/windows/lib/libfmt.a") then
            os.execute("echo Skipping fmt compilation...")
        else
            os.execute("cd libs\\fmt && cmake -G \"MinGW Makefiles\" -S . -B . && make -j" .. os.getenv("NUMBER_OF_PROCESSORS"))
            os.execute("mkdir build\\windows\\lib 2>NUL")
            os.execute("copy libs\\fmt\\libfmt.a build\\windows\\lib\\libfmt.a")
        end
        
        -- sdl -----------------------------------------------------------------------------------------------------------------
project "sdl"
-- @TODO: finish sdl configuration
    filter "system:linux"
        kind "staticlib"
        os.execute("mkdir -p build/sdl")
        os.execute("cd build/sdl && cmake -G \"MinGW Makefiles\" -S ../../libs/sdl -B . && make && cp libSDL2.a ../windows/debug/libSDL2.a")
        os.execute("cd build/sdl && cmake -S ../../libs/sdl -B . && make -j4 && cp libSDL2.a ../linux/debug/libsdl.a && cp libSDL2.a ../linux/profile/libsdl.a && cp libSDL2.a ../linux/release/libsdl.a")
        -- os.execute("cd build/sdl && cmake -S ../../libs/sdl -B . && make -j8 && cp libSDL2.a ../linux/debug/libsdl.a")
        
    filter "system:windows"
        kind "None"
        
        function exists(filePath)
            local file = io.open(filePath, "r")
            if file then
                file:close()
                return true
            else
                return false
            end
        end
        
        if exists("build/windows/lib/libSDL2.a") and exists("build//windows/lib/libSDL2main.a") and exists("build/windows/lib/libSDL2.dll.a") and exists("build/windows/SDL2.dll") then
            os.execute("echo Skipping SDL2 compilation...")
        else
            os.execute("mkdir build\\windows\\sdl")
            os.execute("cd build\\windows\\sdl && cmake -G \"MinGW Makefiles\" -S ../../../libs/sdl -B . && make -j" .. os.getenv("NUMBER_OF_PROCESSORS"))
            os.execute("mkdir build\\windows\\lib 2>NUL")
            os.execute("copy build\\windows\\sdl\\libSDL2.a build\\windows\\lib\\libSDL2.a")
            os.execute("copy build\\windows\\sdl\\libSDL2main.a build\\windows\\lib\\libSDL2main.a")
            os.execute("copy build\\windows\\sdl\\libSDL2.dll.a build\\windows\\lib\\libSDL2.dll.a")
            os.execute("copy build\\windows\\sdl\\SDL2.dll build\\windows\\SDL2.dll")
        end
    