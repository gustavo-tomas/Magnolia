#include "core/application.hpp"

#include "core/logger.hpp"
#include "scripting/scripting_engine.hpp"

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

        // Create the file system
        file_system = std::make_unique<FileSystem>();
        LOG_SUCCESS("FileSystem initialized");

        // Create the job system
        job_system = std::make_unique<JobSystem>(std::thread::hardware_concurrency());
        LOG_SUCCESS("JobSystem initialized");

        // Create the image loader
        image_loader = std::make_unique<ImageLoader>();
        LOG_SUCCESS("ImageLoader initialized");

        // Create the material loader
        material_loader = std::make_unique<MaterialLoader>();
        LOG_SUCCESS("MaterialLoader initialized");

        // Create the model loader
        model_loader = std::make_unique<ModelLoader>();
        LOG_SUCCESS("ModelLoader initialized");

        // Create the shader loader
        shader_loader = std::make_unique<ShaderLoader>();
        LOG_SUCCESS("ShaderLoader initialized");

        // Create the texture manager
        texture_loader = std::make_unique<TextureManager>();
        LOG_SUCCESS("TextureManager initialized");

        // Create the material manager
        material_manager = std::make_unique<MaterialManager>();
        LOG_SUCCESS("MaterialManager initialized");

        // Create the model manager
        model_manager = std::make_unique<ModelManager>();
        LOG_SUCCESS("ModelManager initialized");

        // Create the shader manager
        shader_manager = std::make_unique<ShaderManager>();
        LOG_SUCCESS("ShaderManager initialized");

        // Create the physics engine
        physics_engine = std::make_unique<PhysicsEngine>();
        LOG_SUCCESS("Physics initialized");

        // Create the scripting engine
        ScriptingEngine::initialize();
        LOG_SUCCESS("ScriptingEngine initialized");

        running = true;
    }

    Application::~Application()
    {
        for (auto* layer : layers)
        {
            layer->on_detach();
            delete layer;
        }

        ScriptingEngine::shutdown();
    }

    void Application::run()
    {
        f64 curr_time = 0, last_time = 0, dt = 0;

        while (running)
        {
            // Calculate dt
            curr_time = window->get_time();
            dt = curr_time - last_time;
            last_time = curr_time;

            window->update();

            // Skip rendering if minimized or resizing
            if (window->is_minimized())
            {
                window->sleep(50);
                continue;
            }

            job_system->process_callbacks();

            for (auto* layer : layers)
            {
                layer->on_update(dt);
            }
        }
    }

    void Application::push_layer(Layer* layer)
    {
        layers.push_back(layer);
        layer->on_attach();
    }

    void Application::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BIND_FN(Application::on_window_close));
        dispatcher.dispatch<QuitEvent>(BIND_FN(Application::on_quit));

        renderer->on_event(e);

        for (auto* layer : layers)
        {
            layer->on_event(e);
        }
    }

    void Application::on_quit(QuitEvent& e)
    {
        (void)e;
        running = false;
    }

    void Application::on_window_close(WindowCloseEvent& e)
    {
        (void)e;
        running = false;
    }
};  // namespace mag
