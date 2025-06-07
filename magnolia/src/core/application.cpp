#include "core/application.hpp"

#include "audio/audio_system.hpp"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/types.hpp"
#include "gfx/gfx.hpp"
#include "platform/file_system.hpp"
#include "platform/platform.hpp"
#include "platform/window.hpp"
#include "threads/job_system.hpp"
#include "threads/thread.hpp"
#include "tools/profiler.hpp"

namespace mag
{
    static Application* application = nullptr;

    Application& get_application()
    {
        MAG_ASSERT(application != nullptr, "Application is null");
        return *application;
    }

    Application::Application(const str& config_file_path)
    {
        application = this;

        b8 initialized = true;

        // Initialize the platform subsystem
        initialized = initialized && plat::initialize();

        // Initialize the filesystem subsystem
        initialized = initialized && fs::initialize();

        // Initialize the threading subsystem
        initialized = initialized && thread::initialize();

        // Initialize the audio subsystem
        initialized = initialized && audio::initialize();

        // Read config file

        fs::json config;

        math::uvec2 window_size = WindowOptions::MaxSize;
        math::ivec2 window_position = WindowOptions::CenterPos;
        math::uvec2 screen_resolution = {1280, 720};
        str window_title = "Magnolia";
        str window_icon = "";

        if (fs::read_json_data(config_file_path, config))
        {
            u32 count = 0;
            for (const auto& num : config["WindowSize"])
            {
                if (count >= window_size.length()) break;
                window_size[count++] = num;
            }

            count = 0;
            for (const auto& num : config["WindowPosition"])
            {
                if (count >= window_position.length()) break;
                window_position[count++] = num;
            }

            count = 0;
            for (const auto& num : config["ScreenResolution"])
            {
                if (count >= screen_resolution.length()) break;
                screen_resolution[count++] = num;
            }

            window_title = config["WindowTitle"].get<str>();
            window_icon = config["WindowIcon"].get<str>();
        }

        // Set target frame rate
        set_target_frame_rate(config["TargetFrameRate"].get<f32>());

        const WindowOptions window_options = {BIND_FN(Application::process_event), window_size, window_position,
                                              window_title, window_icon};

        // Initialize the window
        initialized = initialized && window::initialize(window_options);

        // Initialize graphics subsystem
        gfx::GfxOptions gfx_options = {};
        gfx_options.resolution = screen_resolution;
        initialized = initialized && gfx::initialize(gfx_options);

        // Initialize the resource subsystem
        initialized = initialized && resource::initialize();

        if (initialized)
        {
            LOG_SUCCESS("Application initialized");
        }

        else
        {
            MAG_ASSERT(false, "Failed to initialize Application");
        }

        // Set resource load callback
        resource::set_on_resource_loaded_callback(BIND_FN(Application::on_resource_loaded));
    }

    Application::~Application()
    {
        resource::shutdown();
        gfx::shutdown();
        window::shutdown();
        audio::shutdown();
        thread::shutdown();
        fs::shutdown();
        plat::shutdown();
    }

    void Application::run()
    {
        f64 curr_time = 0, last_time = 0, dt = 0;

        running = true;

        while (running)
        {
            // Calculate dt
            curr_time = plat::get_time();
            dt = (curr_time - last_time) / 1000.0;  // convert from ms to seconds
            last_time = curr_time;

            SCOPED_PROFILE("Application");

            window::on_update();

            // Skip rendering if minimized or resizing
            if (window::is_minimized())
            {
                thread::sleep(50);
                continue;
            }

            thread::process_callbacks();

            // Update the user application
            on_update(dt);

            // Delay if needed
            const f64 delay = (1000.0 / target_frame_rate) - (plat::get_time() - last_time);
            if (delay > 0.0 && target_frame_rate > 0.0)
            {
                thread::sleep(delay);
            }
        }
    }

    void Application::on_resource_loaded(const IResource* resource)
    {
        // Send the event to the user.
        if (on_resource_loaded_user_callback != nullptr)
        {
            on_resource_loaded_user_callback(resource);
        }
    }

    void Application::process_event(const Event& e)
    {
        // Process the event internally
        dispatch_event<WindowCloseEvent>(e, BIND_FN(Application::on_window_close));
        dispatch_event<QuitEvent>(e, BIND_FN(Application::on_quit));

        // Send event to be processed by the user application
        on_event(e);
    }

    void Application::process_user_application_event(const Event& e) { process_event(e); }

    void Application::on_quit(const QuitEvent& e)
    {
        (void)e;
        running = false;
    }

    void Application::on_window_close(const WindowCloseEvent& e)
    {
        (void)e;
        running = false;
    }

    void Application::set_target_frame_rate(const f32 frame_rate) { target_frame_rate = frame_rate; }

    void Application::set_on_resource_loaded_callback(const ResourceLoadedCallbackFn& callback)
    {
        on_resource_loaded_user_callback = callback;
    }
};  // namespace mag
