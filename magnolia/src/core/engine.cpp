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
#include "magnolia/tools/console.hpp"

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
        initialized = initialized && console::initialize();

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

        console::shutdown();
        resource::shutdown();
        gfx::shutdown();
        window::shutdown();
        audio::shutdown();
        thread::shutdown();
        fs::shutdown();
        plat::shutdown();
    }

    EngineInitializeOptions read_config_file(const str& file_path)
    {
        EngineInitializeOptions options = {};

        mag::fs::json config;

        if (mag::fs::read_json_data(file_path, config))
        {
            mag::window::WindowOptions& window_options = options.window_options;
            mag::gfx::GfxOptions& gfx_options = options.gfx_options;

            u32 count = 0;
            for (const auto& num : config["WindowSize"])
            {
                if (count >= window_options.size.length())
                {
                    break;
                }
                window_options.size[count++] = num;
            }

            count = 0;
            for (const auto& num : config["WindowPosition"])
            {
                if (count >= window_options.position.length())
                {
                    break;
                }
                window_options.position[count++] = num;
            }

            count = 0;
            for (const auto& num : config["ScreenResolution"])
            {
                if (count >= gfx_options.resolution.length())
                {
                    break;
                }
                gfx_options.resolution[count++] = num;
            }

            window_options.title = config["WindowTitle"].get<str>();
            window_options.window_icon = config["WindowIcon"].get<str>();
            window_options.target_frame_rate = config["TargetFrameRate"].get<i32>();
        }

        return options;
    }
};  // namespace mag
