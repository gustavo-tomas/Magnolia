#include "magnolia/core/application.hpp"

#include "magnolia/audio/audio_system.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/event.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/gfx/gfx.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/platform.hpp"
#include "magnolia/platform/window.hpp"
#include "magnolia/threads/job_system.hpp"
#include "magnolia/threads/thread.hpp"
#include "magnolia/tools/profiler.hpp"

namespace mag
{
    static Application* application = nullptr;

    Application::Application(const ApplicationOptions& options)
    {
        MAG_ASSERT(application == nullptr, "Application was already initialized");

        application = this;

        b8 initialized = true;

        initialized = initialized && plat::initialize();
        initialized = initialized && fs::initialize();
        initialized = initialized && thread::initialize();
        initialized = initialized && audio::initialize();
        initialized = initialized && window::initialize(options.window_options);
        initialized = initialized && gfx::initialize(options.gfx_options);
        initialized = initialized && resource::initialize();

        MAG_ASSERT(initialized, "Failed to initialize Application");

        if (initialized)
        {
            LOG_SUCCESS("Application initialized");
        }

        set_target_frame_rate(options.target_frame_rate);
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

            MAG_SCOPED_PROFILE("Application");

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
