#include "core/application.hpp"

#include <thread>

#include "audio/audio_system.hpp"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/types.hpp"
#include "core/window.hpp"
#include "platform/file_system.hpp"
#include "renderer/renderer.hpp"
#include "resources/resource.hpp"
#include "threads/job_system.hpp"
#include "tools/profiler.hpp"

namespace mag
{
    static Application* application = nullptr;

    Application& get_application()
    {
        MAG_ASSERT(application != nullptr, "Application is null");
        return *application;
    }

    struct Application::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            unique<Window> window;

            b8 running;
            f32 target_frame_rate;
    };

    Application::Application(const str& config_file_path) : impl(new IMPL())
    {
        application = this;

        b8 initialized = true;

        // Initialize the filesystem subsystem
        initialized = initialized && fs::initialize();

        // Initialize the threading subsystem
        initialized = initialized && thread::initialize(std::thread::hardware_concurrency());

        // Initialize the audio subsystem
        initialized = initialized && audio::initialize();

        // Read config file

        fs::json config;

        uvec2 window_size = WindowOptions::MaxSize;
        ivec2 window_position = WindowOptions::CenterPos;
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

            window_title = config["WindowTitle"].get<str>();
            window_icon = config["WindowIcon"].get<str>();
        }

        // Set target frame rate
        set_target_frame_rate(config["TargetFrameRate"].get<f32>());

        // Create the window
        const WindowOptions window_options = {BIND_FN(Application::process_event), window_size, window_position,
                                              window_title, window_icon};

        impl->window = create_unique<Window>(window_options);

        // Initialize graphics subsystem
        initialized = initialized && gfx::initialize(*impl->window);

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
    }

    Application::~Application()
    {
        resource::shutdown();
        gfx::shutdown();
        audio::shutdown();
        thread::shutdown();
        fs::shutdown();
    }

    void Application::run()
    {
        f64 curr_time = 0, last_time = 0, dt = 0;

        impl->running = true;

        while (impl->running)
        {
            // Calculate dt
            curr_time = impl->window->get_time();
            dt = (curr_time - last_time) / 1000.0;  // convert from ms to seconds
            last_time = curr_time;

            SCOPED_PROFILE("Application");

            impl->window->on_update();

            // Skip rendering if minimized or resizing
            if (impl->window->is_minimized())
            {
                impl->window->sleep(50);
                continue;
            }

            thread::process_callbacks();

            // Update the user application
            on_update(dt);

            // Delay if needed
            const f64 delay = (1000.0 / impl->target_frame_rate) - (impl->window->get_time() - last_time);
            if (delay > 0.0 && impl->target_frame_rate > 0.0)
            {
                impl->window->sleep(delay);
            }
        }
    }

    void Application::process_event(const Event& e)
    {
        // Process the event internally
        dispatch_event<WindowCloseEvent>(e, BIND_FN(Application::on_window_close));
        dispatch_event<QuitEvent>(e, BIND_FN(Application::on_quit));

        gfx::on_event(e);

        // Send event to be processed by the user application
        on_event(e);
    }

    void Application::process_user_application_event(const Event& e) { process_event(e); }

    void Application::on_quit(const QuitEvent& e)
    {
        (void)e;
        impl->running = false;
    }

    void Application::on_window_close(const WindowCloseEvent& e)
    {
        (void)e;
        impl->running = false;
    }

    void Application::set_target_frame_rate(const f32 frame_rate) { impl->target_frame_rate = frame_rate; }

    Window& Application::get_window() { return *impl->window; }
};  // namespace mag
