#include "scripting/scripting_engine.hpp"

// @TODO: this is unix only, create an interface for the windows build
#if MAG_PLATFORM_LINUX
    #include <dlfcn.h>
#else
    #error "Unsupported platform"
#endif

#include "core/logger.hpp"
#include "platform/file_system.hpp"

namespace mag
{
    namespace script
    {
        static b8 compile_script(const str& input_file_path, const str& output_file_path,
                                 const std::vector<str>& include_paths, const std::vector<str>& lib_paths,
                                 const std::vector<str>& link_libs);

        void* load_script(const str& file_path)
        {
            const str script_name = fs::path(file_path).stem();
            const str bin_script_file_path = MAG_BUILD_SCRIPT_NAME(script_name);

            // @TODO: see if we can load this from memory
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

        b8 recompile_script(const str& file_path, const b8 force_recompilation)
        {
            const str script_name = fs::path(file_path).stem();
            const str bin_script_file_path = MAG_BUILD_SCRIPT_NAME(script_name);

            if (!force_recompilation && fs::exists(bin_script_file_path))
            {
                return true;
            }

            const std::vector<str> include_paths = {"magnolia/src", "libs/fmt/include"};
            const std::vector<str> lib_paths = {MAG_BUILD_DIR_BIN "fmt", MAG_BUILD_DIR_BIN "magnolia"};
            const std::vector<str> link_libs = {"magnolia", "fmt"};

            if (!compile_script(file_path, bin_script_file_path, include_paths, lib_paths, link_libs))
            {
                LOG_ERROR("Failed to compile script: '{0}'", file_path);
                return false;
            }

            return true;
        }

        static b8 compile_script(const str& input_file_path, const str& output_file_path,
                                 const std::vector<str>& include_paths, const std::vector<str>& lib_paths,
                                 const std::vector<str>& link_libs)
        {
            LOG_INFO("Compiling script '{0}'...", input_file_path);

            // Create directories if they dont exist
            fs::create_directories(fs::path(output_file_path).parent_path());

            // @TODO: for now no optimizations
            str compile_script_cmd = "clang++ -std=c++20 -fPIC -shared -O0";

            // Include paths
            for (const str& path : include_paths)
            {
                compile_script_cmd += " -I" + path;
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

            compile_script_cmd += " " + input_file_path + " -o " + output_file_path;

            // Execute clang
            return system(compile_script_cmd.c_str()) == 0;
        }
    };  // namespace script
};      // namespace mag
