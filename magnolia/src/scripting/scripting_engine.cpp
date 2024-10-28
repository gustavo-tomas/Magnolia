#include "scripting/scripting_engine.hpp"

// @TODO: this is unix only, create an interface for the windows build
#include <dlfcn.h>

#include <filesystem>

#include "core/logger.hpp"

namespace mag
{
    void* ScriptingEngine::load_script(const str& file_path)
    {
        // @TODO: cleanup
        str scripts_bin_folder = "scripts/";
        str extension = ".so";
        str configuration = "_debug";
        {
            const std::filesystem::path cwd = std::filesystem::current_path();
            const str last_folder = cwd.filename().string();
            str system = "linux";

#if defined(_WIN32)
            system = "windows";
            extension = ".dll";
#endif

#if defined(MAG_PROFILE)
            configuration = "_profile";
#elif defined(MAG_RELEASE)
            configuration = "_release";
#endif

            if (last_folder == "Magnolia") scripts_bin_folder = "build/" + system + "/" + scripts_bin_folder;
        }
        // @TODO: cleanup

        const str script_src = std::filesystem::path(file_path).stem();
        const str script_dll = scripts_bin_folder + "lib" + script_src + configuration + extension;

        // @TODO: see if we can load this from memory
        void* handle = dlopen(script_dll.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            LOG_ERROR("Failed to load script '{0}': {1}", script_dll, dlerror());
            return nullptr;
        }

        return handle;
    }

    void ScriptingEngine::unload_script(void* handle)
    {
        if (handle)
        {
            dlclose(handle);
        }
    }

    void* ScriptingEngine::get_symbol(void* handle, const str& name)
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
};  // namespace mag
