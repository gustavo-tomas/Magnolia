#include "magnolia/scripting/scripting_engine.hpp"

#include <unordered_map>

#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/threads/process_manager.hpp"

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
            std::erase_if(state->scripts, [](const auto& item)
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

            // Create directories if they dont exist
            fs::create_directories(fs::path(bin_script_file_path).parent_path());

            const str command = "clang++";

            std::vector<str> args;
            args.reserve(params.compilation_flags.size());

            // Flags
            for (const str& arg : params.compilation_flags)
            {
                args.push_back(arg);
            }

            // Include paths
            for (const str& arg : params.include_paths)
            {
                args.push_back("-I" + arg);
            }

            // Defines
            for (const str& arg : params.defines)
            {
                args.push_back("-D" + arg);
            }

            // Libs paths
            for (const str& arg : params.lib_paths)
            {
                args.push_back("-L" + arg);
            }

            // Link libs
            for (const str& arg : params.link_libs)
            {
                args.push_back("-l" + arg);
            }

            args.push_back(file_path);
            args.push_back("-o" + bin_script_file_path);

            LOG_INFO("Compiling script '{0}'...", file_path);

            // Execute clang
            const b8 result = thread::execute_process(command, args);

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
