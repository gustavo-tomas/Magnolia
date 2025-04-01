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
        void* load_script(const str& file_path)
        {
            // @TODO: cleanup
            str extension = ".so";
            str configuration = "_debug";
            {
#if MAG_PLATFORM_WINDOWS
                extension = ".dll";
#endif

#if MAG_CONFIG_PROFILE
                configuration = "_profile";
#elif MAG_CONFIG_RELEASE
                configuration = "_release";
#endif
            }
            // @TODO: cleanup

            const str script_src = fs::path(file_path).stem();
            const str script_dll = MAG_BUILD_DIR_SCRIPTS "lib" + script_src + configuration + extension;

            // @TODO: see if we can load this from memory
            void* handle = dlopen(script_dll.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (!handle)
            {
                LOG_ERROR("Failed to load script '{0}': {1}", script_dll, dlerror());
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
    };  // namespace script
};      // namespace mag
