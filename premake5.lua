-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "magnolia"
    architecture "x86_64"
    language "c++"
    cppdialect "c++20"
    toolset "gcc"
    configurations { "debug", "profile", "release" }
    location "build"
    staticruntime "on"
    
    -- Output directories
    targetdir ("build/%{cfg.system}/magnolia")
    objdir ("build/%{cfg.system}/obj/%{cfg.buildcfg}/%{prj.name}")

-- @TODO: consistent build folders/output

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
        "fmt"
    }

    libdirs
    { 
        "build/%{cfg.system}/lib"
    }

    filter "system:linux"
        pic "on"
        links
        {
            "vulkan", "sdl"
        }

    filter "system:windows"
        systemversion "latest"

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

-- Skip compilation if already compiled
function exists(filePath)
    local file = io.open(filePath, "r")
    if file then
        file:close()
        return true
    else
        return false
    end
end

-- Execute a shell command
function execute_command(command)
    local file = io.popen(command)
    local output = file:read("*a")
    file:close()
    return output
end

-- Get number of cores (linux only)
function number_of_cores()
    result = tonumber(execute_command("nproc"))
    print("Number of cores:", result)
    return result
end

-- vulkan --------------------------------------------------------------------------------------------------------------
project "vulkan"
    kind "none"

    if os.host() == "windows" then
        -- @TODO
        os.execute("MISSING WINDOWS VULKAN DLL")

    elseif os.host() == "linux" then
        if exists("build/linux/magnolia/libvulkan.so") and
           exists("build/linux/magnolia/libvulkan.so.1") and
           exists("build/linux/magnolia/libvulkan.so.1.3.268") then
            
            os.execute("echo Skipping vulkan copy commands...")
        else
            os.execute("mkdir -p build/linux/lib")
            os.execute("mkdir -p build/linux/magnolia")
            os.execute("cp ext/linux/libvulkan.so build/linux/magnolia/libvulkan.so")
            os.execute("cp ext/linux/libvulkan.so.1 build/linux/magnolia/libvulkan.so.1")
            os.execute("cp ext/linux/libvulkan.so.1.3.268 build/linux/magnolia/libvulkan.so.1.3.268")
        end
    end

-- fmt -----------------------------------------------------------------------------------------------------------------
project "fmt"
    kind "none"
    targetextension ".a"
    targetprefix "lib"

    if os.host() == "windows" then
        if exists("build/windows/lib/libfmt.a") then
            os.execute("echo Skipping fmt compilation...")
        else
            os.execute("cd libs\\fmt && cmake -G \"MinGW Makefiles\" -S . -B . && make -j" .. os.getenv("NUMBER_OF_PROCESSORS"))
            os.execute("mkdir build\\windows\\lib 2>NUL")
            os.execute("copy libs\\fmt\\libfmt.a build\\windows\\lib\\libfmt.a")
        end

    elseif os.host() == "linux" then
        if exists("build/linux/lib/libfmt.a") then
            os.execute("echo Skipping fmt compilation...")
        else
            os.execute("mkdir -p build/linux/fmt")
            os.execute("cd build/linux/fmt && cmake -S ../../../libs/fmt -B . && make -j" .. number_of_cores())
            os.execute("cp build/linux/fmt/libfmt.a build/linux/lib/libfmt.a")
        end
    end
        
-- sdl -----------------------------------------------------------------------------------------------------------------
project "sdl"
    kind "none"
    
    if os.host() == "windows" then
        if exists("build/windows/lib/libSDL2.a") and exists("build/windows/lib/libSDL2main.a") and exists("build/windows/lib/libSDL2.dll.a") and exists("build/windows/SDL2.dll") then
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

    elseif os.host() == "linux" then
        if exists("build/linux/lib/libsdl.a") and exists("build/linux/lib/libSDL2main.a") then
            os.execute("echo Skipping SDL2 compilation...")
        else
            os.execute("mkdir -p build/linux/sdl")
            os.execute("cd build/linux/sdl && cmake -S ../../../libs/sdl -B . && make -j" .. number_of_cores())
            os.execute("cp build/linux/sdl/libSDL2.a build/linux/lib/libsdl.a")
            os.execute("cp build/linux/sdl/libSDL2main.a build/linux/lib/libSDL2main.a")
        end
    end
    