#include "magnolia/scripting/scripting_engine.hpp"

// @TODO: this is unix only, create an interface for the windows build
#if MAG_PLATFORM_LINUX
    #include <dlfcn.h>
#else
    #error "Unsupported platform"
#endif

#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"

namespace mag
{
    namespace script
    {
        void* load_script(const str& file_path)
        {
            const str script_name = fs::path(file_path).stem();
            const str bin_script_file_path = MAG_BUILD_SCRIPT_NAME(script_name);

            void* handle = dlopen(bin_script_file_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (!handle)
            {
                LOG_ERROR("Failed to load script '{0}': {1}", bin_script_file_path, dlerror());
                return nullptr;
            }

            return handle;
        }

        void unload_script(void* handle)
        {
            if (handle)
            {
                dlclose(handle);
            }
        }

        void* get_symbol(void* handle, const str& name)
        {
            if (!handle)
            {
                LOG_ERROR("Handle is nullptr");
                return nullptr;
            }

            void* symbol = dlsym(handle, name.c_str());

            if (!symbol)
            {
                LOG_ERROR("Failed to load script symbols '{0}': {1}", name, dlerror());
                return nullptr;
            }

            return symbol;
        }

        b8 compile_script(const RecompileScriptParams& params)
        {
            const str& file_path = params.file_path;
            const b8 force_recompilation = params.force_recompilation;

            const str script_name = fs::path(file_path).stem();
            const str bin_script_file_path = MAG_BUILD_SCRIPT_NAME(script_name);

            if (!force_recompilation && fs::exists(bin_script_file_path))
            {
                return true;
            }

            const str compilation_flags = params.compilation_flags;
            const std::vector<str> include_paths = params.include_paths;
            const std::vector<str> lib_paths = params.lib_paths;
            const std::vector<str> link_libs = params.link_libs;
            std::vector<str> defines = params.defines;
            defines.push_back("FMT_USE_CONSTEVAL=0");  // @TODO: remove when fmt is removed

            // Create directories if they dont exist
            fs::create_directories(fs::path(bin_script_file_path).parent_path());

            str compile_script_cmd = "clang++ " + compilation_flags;

            // Include paths
            for (const str& path : include_paths)
            {
                compile_script_cmd += " -I" + path;
            }

            // Defines
            for (const str& def : defines)
            {
                compile_script_cmd += " -D" + def;
            }

            // Libs paths
            for (const str& path : lib_paths)
            {
                compile_script_cmd += " -L" + path;
            }

            // Link libs
            for (const str& lib : link_libs)
            {
                compile_script_cmd += " -l" + lib;
            }

            compile_script_cmd += " " + file_path + " -o " + bin_script_file_path;

            LOG_INFO("Compiling script '{0}'...", file_path);

            // Execute clang
            const b8 result = system(compile_script_cmd.c_str()) == 0;

            if (!result)
            {
                LOG_ERROR("Failed to compile script: '{0}'", file_path);
                return false;
            }

            LOG_SUCCESS("Finished compiling script: '{0}'", file_path);

            return true;
        }
    };  // namespace script
};      // namespace mag
