#include "magnolia/core/engine.hpp"

#include "magnolia/audio/audio_system.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/gfx/gfx.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/platform.hpp"
#include "magnolia/platform/window.hpp"
#include "magnolia/resources/resource.hpp"
#include "magnolia/threads/thread.hpp"

namespace mag
{
    static b8 initialized = false;

    b8 initialize(const EngineInitializeOptions& options)
    {
        MAG_ASSERT(!initialized, "Engine was already initialized");

        initialized = true;

        initialized = initialized && plat::initialize();
        initialized = initialized && fs::initialize();
        initialized = initialized && thread::initialize();
        initialized = initialized && audio::initialize();
        initialized = initialized && window::initialize(options.window_options);
        initialized = initialized && gfx::initialize(options.gfx_options);
        initialized = initialized && resource::initialize();

        MAG_ASSERT(initialized, "Failed to initialize Engine");

        if (initialized)
        {
            LOG_SUCCESS("Engine initialized");
        }

        else
        {
            LOG_ERROR("Failed to initialize Engine");
        }

        return initialized;
    }

    void shutdown()
    {
        if (!initialized)
        {
            return;
        }

        resource::shutdown();
        gfx::shutdown();
        window::shutdown();
        audio::shutdown();
        thread::shutdown();
        fs::shutdown();
        plat::shutdown();
    }
};  // namespace mag
