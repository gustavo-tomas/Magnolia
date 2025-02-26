#include "core/application.hpp"

#include "core/assert.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/window.hpp"
#include "platform/file_dialog.hpp"
#include "platform/file_system.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "resources/image.hpp"
#include "resources/material.hpp"
#include "resources/model.hpp"
#include "threads/job_system.hpp"
#include "tools/profiler.hpp"

namespace mag
{
    static Application* application = nullptr;

    Application& get_application()
    {
        ASSERT(application != nullptr, "Application is null");
        return *application;
    }

    struct Application::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            unique<Window> window;
            unique<Renderer> renderer;
            unique<FileWatcher> file_watcher;
            unique<JobSystem> job_system;
            unique<TextureManager> texture_loader;
            unique<MaterialManager> material_manager;
            unique<ModelManager> model_manager;
            unique<ShaderManager> shader_manager;

            b8 running;
            f32 target_frame_rate;
    };

    Application::Application(const str& config_file_path) : impl(new IMPL())
    {
        application = this;

        // Remember that smart pointers are destroyed in the reverse order of creation

        // Read config file

        json config;

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
        LOG_SUCCESS("Window initialized");

        // Create the renderer
        impl->renderer = create_unique<Renderer>(*impl->window);
        LOG_SUCCESS("Renderer initialized");

        // Create the file watcher
        impl->file_watcher = create_unique<FileWatcher>();
        LOG_SUCCESS("FileWatcher initialized");

        // Create the job system
        impl->job_system = create_unique<JobSystem>(std::thread::hardware_concurrency());
        LOG_SUCCESS("JobSystem initialized");

        // Create the texture manager
        impl->texture_loader = create_unique<TextureManager>();
        LOG_SUCCESS("TextureManager initialized");

        // Create the material manager
        impl->material_manager = create_unique<MaterialManager>();
        LOG_SUCCESS("MaterialManager initialized");

        // Create the model manager
        impl->model_manager = create_unique<ModelManager>();
        LOG_SUCCESS("ModelManager initialized");

        // Create the shader manager
        impl->shader_manager = create_unique<ShaderManager>();
        LOG_SUCCESS("ShaderManager initialized");

        // Initialize file dialogs
        if (FileDialog::initialize())
        {
            LOG_SUCCESS("FileDialog initialized");
        }
    }

    Application::~Application() { FileDialog::shutdown(); }

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

            impl->job_system->process_callbacks();

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

        impl->renderer->on_event(e);

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
    Renderer& Application::get_renderer() { return *impl->renderer; }
    FileWatcher& Application::get_file_watcher() { return *impl->file_watcher; }
    JobSystem& Application::get_job_system() { return *impl->job_system; }
    TextureManager& Application::get_texture_manager() { return *impl->texture_loader; }
    MaterialManager& Application::get_material_manager() { return *impl->material_manager; }
    ModelManager& Application::get_model_manager() { return *impl->model_manager; }
    ShaderManager& Application::get_shader_manager() { return *impl->shader_manager; }
};  // namespace mag
