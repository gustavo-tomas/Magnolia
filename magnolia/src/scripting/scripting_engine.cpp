#include "magnolia/scripting/scripting_engine.hpp"

#include <unordered_map>

#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/platform/file_system.hpp"

// @TODO: this is unix only, create an interface for the windows build
#if MAG_PLATFORM_LINUX
    #include <dlfcn.h>
#else
    #error "Unsupported platform"
#endif

namespace mag
{
    namespace script
    {
        struct State
        {
                std::unordered_map<ScriptHandle, void*> scripts;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            return state != nullptr;
        }

        void shutdown()
        {
            std::erase_if(state->scripts,
                          [](const auto& item)
                          {
                              const auto& [key, value] = item;
                              dlclose(value);
                              return true;
                          });

            delete state;
        }

        static ScriptHandle create_handle()
        {
            static u32 counter = 0;
            return counter++;
        }

        ScriptHandle load_script(const str& file_path)
        {
            const str script_name = fs::path(file_path).stem();
            const str bin_script_file_path = MAG_BUILD_SCRIPT_NAME(script_name);

            void* script = dlopen(bin_script_file_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (script == nullptr)
            {
                LOG_ERROR("Failed to load script '{0}': {1}", bin_script_file_path, dlerror());
                return Invalid_ID;
            }

            const ScriptHandle handle = create_handle();

            state->scripts[handle] = script;

            return handle;
        }

        b8 unload_script(const ScriptHandle handle)
        {
            auto it = state->scripts.find(handle);
            if (it == state->scripts.end())
            {
                return false;
            }

            const b8 res = dlclose(it->second) == 0;
            state->scripts.erase(it);

            return res;
        }

        void* get_symbol(const ScriptHandle handle, const str& name)
        {
            auto it = state->scripts.find(handle);
            if (it == state->scripts.end())
            {
                LOG_ERROR("Invalid handle");
                return nullptr;
            }

            void* symbol = dlsym(it->second, name.c_str());

            if (symbol == nullptr)
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

            // Create directories if they dont exist
            fs::create_directories(fs::path(bin_script_file_path).parent_path());

            str compile_script_cmd = "clang++ " + compilation_flags;

            // Include paths
            compile_script_cmd = std::accumulate(include_paths.begin(), include_paths.end(), compile_script_cmd,
                                                 [](const str& cmd, const str& arg) { return cmd + " -I" + arg; });

            // Defines
            compile_script_cmd = std::accumulate(defines.begin(), defines.end(), compile_script_cmd,
                                                 [](const str& cmd, const str& arg) { return cmd + " -D" + arg; });

            // Libs paths
            compile_script_cmd = std::accumulate(lib_paths.begin(), lib_paths.end(), compile_script_cmd,
                                                 [](const str& cmd, const str& arg) { return cmd + " -L" + arg; });

            // Link libs
            compile_script_cmd = std::accumulate(link_libs.begin(), link_libs.end(), compile_script_cmd,
                                                 [](const str& cmd, const str& arg) { return cmd + " -l" + arg; });

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
};  // namespace mag
