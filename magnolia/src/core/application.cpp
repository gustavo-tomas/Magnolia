#include "core/application.hpp"

#include "core/logger.hpp"
#include "platform/file_dialog.hpp"
#include "scripting/scripting_engine.hpp"
#include "tools/profiler.hpp"

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

        window = create_unique<Window>(window_options);
        LOG_SUCCESS("Window initialized");

        // Create the renderer
        renderer = create_unique<Renderer>(*window);
        LOG_SUCCESS("Renderer initialized");

        // Create the file system
        file_system = create_unique<FileSystem>();
        LOG_SUCCESS("FileSystem initialized");

        // Create the job system
        job_system = create_unique<JobSystem>(std::thread::hardware_concurrency());
        LOG_SUCCESS("JobSystem initialized");

        // Create the image loader
        image_loader = create_unique<ImageLoader>();
        LOG_SUCCESS("ImageLoader initialized");

        // Create the material loader
        material_loader = create_unique<MaterialLoader>();
        LOG_SUCCESS("MaterialLoader initialized");

        // Create the model loader
        model_loader = create_unique<ModelLoader>();
        LOG_SUCCESS("ModelLoader initialized");

        // Create the shader loader
        shader_loader = create_unique<ShaderLoader>();
        LOG_SUCCESS("ShaderLoader initialized");

        // Create the texture manager
        texture_loader = create_unique<TextureManager>();
        LOG_SUCCESS("TextureManager initialized");

        // Create the material manager
        material_manager = create_unique<MaterialManager>();
        LOG_SUCCESS("MaterialManager initialized");

        // Create the model manager
        model_manager = create_unique<ModelManager>();
        LOG_SUCCESS("ModelManager initialized");

        // Create the shader manager
        shader_manager = create_unique<ShaderManager>();
        LOG_SUCCESS("ShaderManager initialized");

        // Create the physics engine
        physics_engine = create_unique<PhysicsEngine>();
        LOG_SUCCESS("Physics initialized");

        // Create the scripting engine
        ScriptingEngine::initialize();
        LOG_SUCCESS("ScriptingEngine initialized");

        // Initialize file dialogs
        if (FileDialog::initialize())
        {
            LOG_SUCCESS("FileDialog initialized");
        }

        running = true;
    }

    Application::~Application()
    {
        for (auto* layer : layers)
        {
            layer->on_detach();
            delete layer;
        }

        FileDialog::shutdown();
        ScriptingEngine::shutdown();
    }

    void Application::run()
    {
        f64 curr_time = 0, last_time = 0, dt = 0;

        while (running)
        {
            // Calculate dt
            curr_time = window->get_time();
            dt = (curr_time - last_time) / 1000.0;  // convert from ms to seconds
            last_time = curr_time;

            SCOPED_PROFILE("Application");

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
