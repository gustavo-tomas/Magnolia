#include "core/application.hpp"

#include "audio/audio_system.hpp"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/types.hpp"
#include "core/window.hpp"
#include "platform/file_system.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"
#include "resources/audio.hpp"
#include "resources/font.hpp"
#include "resources/material.hpp"
#include "resources/model.hpp"
#include "resources/resource.hpp"
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

        const WindowOptions window_options = {BIND_FN(Application::process_event), window_size, window_position,
                                              window_title, window_icon};

        // Initialize the window
        initialized = initialized && window::initialize(window_options);

        // Initialize graphics subsystem
        initialized = initialized && gfx::initialize();

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
        resource::set_on_resource_loaded_callback(
            [](const IResource* resource)
            {
                // Upload texture data to the GPU
                if (const Image* image = dynamic_cast<const Image*>(resource))
                {
                    gfx::update_image(image);
                }

                // Upload model data to the GPU
                else if (const Model* model = dynamic_cast<const Model*>(resource))
                {
                    gfx::upload_model(model);
                }

                // Upload font data to the GPU
                else if (const Font* font = dynamic_cast<const Font*>(resource))
                {
                    for (const auto& [c, character] : font->characters)
                    {
                        if (!character.data.empty())
                        {
                            gfx::upload_image(&character.texture, SamplerAddressMode::ClampToEdge);
                        }
                    }
                }

                else if (const Material* material = dynamic_cast<const Material*>(resource))
                {
                }

                else if (const Audio* audio = dynamic_cast<const Audio*>(resource))
                {
                }
            });
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
        running = false;
    }

    void Application::on_window_close(const WindowCloseEvent& e)
    {
        (void)e;
        running = false;
    }

    void Application::set_target_frame_rate(const f32 frame_rate) { target_frame_rate = frame_rate; }
};  // namespace mag
