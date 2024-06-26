-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "magnolia"
    architecture "x86_64"
    language "c++"
    cppdialect "c++20"
    toolset "gcc"
    configurations {"debug", "profile", "release"}
    location "build"
    staticruntime "on"
    
    -- Output directories
    targetdir ("build/%{cfg.system}/%{prj.name}")
    objdir ("build/%{cfg.system}/obj/%{cfg.buildcfg}/%{prj.name}")
    
    libdir = ""
    if os.host() == "linux" then
        libdir = "build/linux/lib"
    elseif os.host() == "windows" then
        libdir = "build/windows/lib"
    end

    lib_includes = 
    {
        "libs",
        "libs/sdl/include",
        "libs/fmt/include",
        "libs/vulkan/include",
        "libs/vma/include",
        "libs/assimp/include",
        "libs/imgui",
        "libs/imguizmo",
        "libs/glm",
        "libs/stb",
        "libs/spirv_reflect",
        "libs/json/single_include",
        "libs/meshoptimizer/src",
        "libs/bullet/src",

        -- @TODO: this is necessary because 'config.h' is generated by the cmake
        "build/linux/assimp/include"
    }

    lib_links = 
    {
        "fmt", "imgui", "imguizmo", "imgui_file_dialog", "assimp", "meshoptimizer", 
        "BulletDynamics", "BulletInverseDynamics", "BulletCollision",
        "Bullet3Common", "Bullet3Dynamics", "Bullet3Collision", "Bullet3Geometry", 
        "BulletLinearMath"
    }

-- @TODO: consistent build folders/output
-- @TODO: fix windows build

-- Engine --------------------------------------------------------------------------------------------------------------
project "magnolia"
    targetname ("%{prj.name}_%{cfg.buildcfg}")
    kind "staticlib"
    
    files
    {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
        "libs/spirv_reflect/spirv_reflect.h",
        "libs/spirv_reflect/spirv_reflect.cpp",
        "libs/json/single_include/nlohmann/json.hpp"
    }

    includedirs
    {
        "%{prj.name}/src",

        lib_includes
    }

    defines
    {
        "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC"
    }

    links
    {
        lib_links
    }

    libdirs
    { 
        libdir
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
        buildoptions { "-Wall", "-Wextra", "-Werror" }
        defines { "MAG_DEBUG", "MAG_ASSERTIONS_ENABLED" }
        symbols "on" -- '-g'
        optimize "off" -- '-O0'
        runtime "debug"

    filter "configurations:profile"
        buildoptions { "-Werror" }
        defines { "MAG_PROFILE" }
        symbols "off"
        optimize "on" -- '-O2'
        runtime "release"

    filter "configurations:release"
        buildoptions { "-Werror" }
        defines { "MAG_RELEASE", "MAG_ASSERTIONS_ENABLED" } -- @TODO: fix this
        symbols "off"
        optimize "full" -- '-O3'
        runtime "release"

-- Client Application --------------------------------------------------------------------------------------------------
project "sprout"
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
        "magnolia/src",

        lib_includes
    }

    libdirs
    { 
        libdir
    }

    links
    {
        "magnolia", lib_links
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
        buildoptions { "-Wall", "-Wextra", "-Werror" }
        defines { "MAG_DEBUG", "MAG_ASSERTIONS_ENABLED" }
        symbols "on" -- '-g'
        optimize "off" -- '-O0'
        runtime "debug"

    filter "configurations:profile"
        buildoptions { "-Werror" }
        defines { "MAG_PROFILE" }
        symbols "off"
        optimize "on" -- '-O2'
        runtime "release"

    filter "configurations:release"
        buildoptions { "-Werror" }
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
        if exists("build/windows/magnolia/vulkan-1.dll") then
            os.execute("echo Skipping vulkan copy commands...")
        else
            os.execute("mkdir build\\windows\\magnolia 2>NUL")
            os.execute("cp ext/windows/vulkan-1.dll build/windows/magnolia/vulkan-1.dll")
        end

    elseif os.host() == "linux" then
        if exists(libdir .. "/libvulkan.so") and
           exists(libdir .. "/libvulkan.so.1") and
           exists(libdir .. "/libvulkan.so.1.3.268") then
            
            os.execute("echo Skipping vulkan copy commands...")
        else
            os.execute("mkdir -p " .. libdir)
            os.execute("mkdir -p build/linux/magnolia")
            os.execute("cp ext/linux/libvulkan.so " .. libdir .. "/libvulkan.so")
            os.execute("cp ext/linux/libvulkan.so.1 " .. libdir .. "/libvulkan.so.1")
            os.execute("cp ext/linux/libvulkan.so.1.3.268 " .. libdir .. "/libvulkan.so.1.3.268")
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
            os.execute("mkdir build\\windows\\fmt 2>NUL")
            os.execute("cd build\\windows\\fmt && cmake -G \"MinGW Makefiles\" ../../../libs/fmt -B . && make -j" .. os.getenv("NUMBER_OF_PROCESSORS"))
            os.execute("mkdir build\\windows\\lib 2>NUL")
            os.execute("copy build\\windows\\fmt\\libfmt.a build\\windows\\lib\\libfmt.a")
        end

    elseif os.host() == "linux" then
        if exists(libdir .. "/libfmt.a") then
            os.execute("echo Skipping fmt compilation...")
        else
            os.execute("mkdir -p build/linux/fmt")
            os.execute("cd build/linux/fmt && cmake -S ../../../libs/fmt -B . && make -j" .. number_of_cores())
            os.execute("cp build/linux/fmt/libfmt.a " .. libdir .. "/libfmt.a")
        end
    end
        
-- sdl -----------------------------------------------------------------------------------------------------------------
project "sdl"
    kind "none"
    
    if os.host() == "windows" then
        if exists("build/windows/lib/libSDL2.a") and exists("build/windows/lib/libSDL2main.a") and exists("build/windows/lib/libSDL2.dll.a") and exists("build/windows/magnolia/SDL2.dll") then
            os.execute("echo Skipping SDL2 compilation...")
        else
            os.execute("mkdir build\\windows\\magnolia 2>NUL")
            os.execute("mkdir build\\windows\\sdl 2>NUL")
            os.execute("cd build\\windows\\sdl && cmake -G \"MinGW Makefiles\" -S ../../../libs/sdl -B . && make -j" .. os.getenv("NUMBER_OF_PROCESSORS"))
            os.execute("mkdir build\\windows\\lib 2>NUL")
            os.execute("copy build\\windows\\sdl\\libSDL2.a build\\windows\\lib\\libSDL2.a")
            os.execute("copy build\\windows\\sdl\\libSDL2main.a build\\windows\\lib\\libSDL2main.a")
            os.execute("copy build\\windows\\sdl\\libSDL2.dll.a build\\windows\\lib\\libSDL2.dll.a")
            os.execute("copy build\\windows\\sdl\\SDL2.dll build\\windows\\magnolia\\SDL2.dll")
        end

    elseif os.host() == "linux" then
        if exists(libdir .. "/libsdl.a") and exists(libdir .. "/libSDL2main.a") then
            os.execute("echo Skipping SDL2 compilation...")
        else
            os.execute("mkdir -p build/linux/sdl")
            os.execute("cd build/linux/sdl && cmake -S ../../../libs/sdl -B . && make -j" .. number_of_cores())
            os.execute("cp build/linux/sdl/libSDL2.a " .. libdir .. "/libsdl.a")
            os.execute("cp build/linux/sdl/libSDL2main.a " .. libdir .. "/libSDL2main.a")
        end
    end

-- imgui ---------------------------------------------------------------------------------------------------------------
project "imgui"
	kind "staticlib"
	language "c++"
	cppdialect "c++20"

	targetdir (libdir)
    objdir ("build/%{cfg.system}/%{prj.name}/%{cfg.buildcfg}")

	includedirs { ".", "libs/imgui", "libs/sdl/include", "libs/vulkan/include"}

    if os.host() == "windows" then
        os.execute("mkdir build\\windows\\imgui 2>NUL")
    end

	files
	{
		"libs/imgui/imgui.h",
		"libs/imgui/imconfig.h",
		"libs/imgui/imgui_internal.h",
		"libs/imgui/imstb_rectpack.h",
		"libs/imgui/imstb_textedit.h",
		"libs/imgui/imstb_truetype.h",
		"libs/imgui/imgui.cpp",
		"libs/imgui/imgui_draw.cpp",
		"libs/imgui/imgui_tables.cpp",
		"libs/imgui/imgui_widgets.cpp",
		"libs/imgui/imgui_demo.cpp",

		"libs/imgui/backends/imgui_impl_sdl2.h",
		"libs/imgui/backends/imgui_impl_vulkan.h",
		"libs/imgui/backends/imgui_impl_sdl2.cpp",
		"libs/imgui/backends/imgui_impl_vulkan.cpp"
	}

	filter "system:linux"
		pic "on"
		systemversion "latest"
		staticruntime "on"
        
-- imguizmo ------------------------------------------------------------------------------------------------------------
project "imguizmo"
	kind "staticlib"
	language "c++"
	cppdialect "c++20"

	targetdir (libdir)
    objdir ("build/%{cfg.system}/%{prj.name}/%{cfg.buildcfg}")

	includedirs { ".", "libs/imguizmo", "libs/imgui" }

    if os.host() == "windows" then
        os.execute("mkdir build\\windows\\imguizmo 2>NUL")
    end

	files
	{
		"libs/imguizmo/ImGuizmo.h",
		"libs/imguizmo/ImGuizmo.cpp"
	}

	filter "system:linux"
		pic "on"
		systemversion "latest"
		staticruntime "on"

-- assimp --------------------------------------------------------------------------------------------------------------
project "assimp"
    kind "none"
    
    if os.host() == "windows" then
        os.execute("echo @TODO WINDOWS ASSIMP")

    elseif os.host() == "linux" then
        if exists(libdir .. "/libassimp.so") and
           exists(libdir .. "/libassimp.so.5") and
           exists(libdir .. "/libassimp.so.5.3.0") then
            os.execute("echo Skipping assimp compilation...")

        else
            os.execute("mkdir -p build/linux/assimp")
            os.execute("cd build/linux/assimp && cmake -S ../../../libs/assimp -B . && make -j" .. number_of_cores())
            os.execute("cp build/linux/assimp/bin/libassimp.so " .. libdir .. "/libassimp.so")
            os.execute("cp build/linux/assimp/bin/libassimp.so.5 " .. libdir .. "/libassimp.so.5")
            os.execute("cp build/linux/assimp/bin/libassimp.so.5.3.0 " .. libdir .. "/libassimp.so.5.3.0")
        end
    end

-- meshoptimizer -------------------------------------------------------------------------------------------------------
project "meshoptimizer"
    kind "none"
    
    if os.host() == "windows" then
        os.execute("echo @TODO WINDOWS MESHOPTIMIZER")

    elseif os.host() == "linux" then
        if exists(libdir .. "/libmeshoptimizer.a") then
            os.execute("echo Skipping meshoptimizer compilation...")
        else
            os.execute("mkdir -p build/linux/meshoptimizer")
            os.execute("cd build/linux/meshoptimizer && cmake -S ../../../libs/meshoptimizer -B . && make -j" .. number_of_cores())
            os.execute("cp build/linux/meshoptimizer/libmeshoptimizer.a " .. libdir .. "/libmeshoptimizer.a")
        end
    end

-- bullet --------------------------------------------------------------------------------------------------------------
project "bullet"
    kind "none"
    
    -- @TODO: this cmake also compiles the examples. We want to compile only the static libs.

    if os.host() == "windows" then
        os.execute("echo @TODO WINDOWS BULLET")

    elseif os.host() == "linux" then
        if exists(libdir .. "/libBulletDynamics.a") and
           exists(libdir .. "/libBulletInverseDynamics.a") and
           exists(libdir .. "/libBulletCollision.a") and
           exists(libdir .. "/libBullet3Common.a") and
           exists(libdir .. "/libBullet3Dynamics.a") and
           exists(libdir .. "/libBullet3Collision.a") and
           exists(libdir .. "/libBullet3Geometry.a") and
           exists(libdir .. "/libBulletLinearMath.a") then

            os.execute("echo Skipping bullet compilation...")
        else
            os.execute("mkdir -p build/linux/bullet")
            os.execute("cd build/linux/bullet && cmake -S ../../../libs/bullet -B . && make -j" .. number_of_cores())

            os.execute("cp build/linux/bullet/src/BulletDynamics/libBulletDynamics.a " .. libdir .. "/libBulletDynamics.a")
            os.execute("cp build/linux/bullet/src/BulletInverseDynamics/libBulletInverseDynamics.a " .. libdir .. "/libBulletInverseDynamics.a")
            os.execute("cp build/linux/bullet/src/BulletCollision/libBulletCollision.a " .. libdir .. "/libBulletCollision.a")
            os.execute("cp build/linux/bullet/src/Bullet3Common/libBullet3Common.a " .. libdir .. "/libBullet3Common.a")
            os.execute("cp build/linux/bullet/src/Bullet3Dynamics/libBullet3Dynamics.a " .. libdir .. "/libBullet3Dynamics.a")
            os.execute("cp build/linux/bullet/src/Bullet3Collision/libBullet3Collision.a " .. libdir .. "/libBullet3Collision.a")
            os.execute("cp build/linux/bullet/src/Bullet3Geometry/libBullet3Geometry.a " .. libdir .. "/libBullet3Geometry.a")
            os.execute("cp build/linux/bullet/src/LinearMath/libLinearMath.a " .. libdir .. "/libBulletLinearMath.a")
        end
    end

-- imgui file dialog ---------------------------------------------------------------------------------------------------
project "imgui_file_dialog"
    kind "staticlib"
    language "c++"
    cppdialect "c++20"

    targetdir (libdir)
    objdir ("build/%{cfg.system}/%{prj.name}/%{cfg.buildcfg}")

    includedirs { ".", "libs/imgui_file_dialog", "libs/imgui" }

    if os.host() == "windows" then
        os.execute("mkdir build\\windows\\imgui_file_dialog 2>NUL")
    end

    files
    {
        "libs/imgui_file_dialog/ImGuiFileDialog.h",
        "libs/imgui_file_dialog/ImGuiFileDialogConfig.h",
        "libs/imgui_file_dialog/ImGuiFileDialog.cpp"
    }

    filter "system:linux"
        pic "on"
        systemversion "latest"
        staticruntime "on"
