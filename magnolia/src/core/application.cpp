#include "core/application.hpp"

#include "core/logger.hpp"

namespace mag
{
    static Application* application = nullptr;

    Application& get_application()
    {
        ASSERT(application != nullptr, "Application is null");
        return *application;
    }

    Application::Application(const ApplicationOptions& options)
    {
        application = this;

        // Remember that smart pointers are destroyed in the reverse order of creation

        // Create the window
        const WindowOptions window_options = {BIND_FN(Application::on_event), options.size, options.position,
                                              options.title};

        window = std::make_unique<Window>(window_options);
        LOG_SUCCESS("Window initialized");

        // Create the renderer
        renderer = std::make_unique<Renderer>(*window);
        LOG_SUCCESS("Renderer initialized");

        // Create the model loader
        model_loader = std::make_unique<ModelLoader>();
        LOG_SUCCESS("ModelLoader initialized");

        // Create the texture loader
        texture_loader = std::make_unique<TextureLoader>();
        LOG_SUCCESS("TextureLoader initialized");

        // Create the editor
        editor = std::make_unique<Editor>(BIND_FN(Application::on_event));
        LOG_SUCCESS("Editor initialized");

        running = true;
    }

    Application::~Application() {}

    void Application::run()
    {
        u64 curr_time = 0, last_time = 0, frame_counter = 0;
        f64 time_counter = 0.0, dt = 0.0;

        while (running)
        {
            // Calculate dt
            curr_time = window->get_time();
            dt = curr_time - last_time;
            last_time = curr_time;

            frame_counter++;
            time_counter += dt;
            if (time_counter >= 1000.0)
            {
                LOG_INFO("CPU: {0:.3f} ms/frame - {1} fps", 1000.0 / static_cast<f64>(frame_counter), frame_counter);
                frame_counter = time_counter = 0;
            }

            window->update();

            // Skip rendering if minimized
            if (window->is_minimized())
            {
                window->sleep(100);
                continue;
            }

            active_scene->update(dt);

            editor->update();

            renderer->update(active_scene->get_camera(), *editor, active_scene->get_render_pass(),
                             active_scene->get_models());
        }
    }

    void Application::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BIND_FN(Application::on_window_close));
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Application::on_window_resize));

        active_scene->on_event(e);
        editor->on_event(e);
    }

    void Application::on_window_close(WindowCloseEvent& e)
    {
        (void)e;
        running = false;
    }

    void Application::on_window_resize(WindowResizeEvent& e)
    {
        const uvec2 size = {e.width, e.height};

        renderer->on_resize(size);
    }
};  // namespace mag
