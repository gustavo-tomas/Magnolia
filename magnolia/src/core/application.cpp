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

        // Create the texture manager
        texture_loader = std::make_unique<TextureManager>();
        LOG_SUCCESS("TextureManager initialized");

        // Create the material manager
        material_loader = std::make_unique<MaterialManager>();
        LOG_SUCCESS("MaterialManager initialized");

        // Create the model manager
        model_loader = std::make_unique<ModelManager>();
        LOG_SUCCESS("ModelManager initialized");

        // Create the shader manager
        shader_loader = std::make_unique<ShaderManager>();
        LOG_SUCCESS("ShaderManager initialized");

        // Create the editor
        editor = std::make_unique<Editor>(BIND_FN(Application::on_event));
        LOG_SUCCESS("Editor initialized");

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
        active_scene.reset();
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

            physics_engine->update(dt);

            active_scene->update(dt);

            editor->update();

            renderer->update(*active_scene, *editor);
        }
    }

    void Application::set_active_scene(Scene* scene)
    {
        active_scene = std::unique_ptr<Scene>(scene);
        physics_engine->on_simulation_start();
    }

    void Application::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(BIND_FN(Application::on_window_close));
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Application::on_window_resize));
        dispatcher.dispatch<QuitEvent>(BIND_FN(Application::on_quit));

        active_scene->on_event(e);
        editor->on_event(e);
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

    void Application::on_window_resize(WindowResizeEvent& e)
    {
        const uvec2 size = {e.width, e.height};

        renderer->on_resize(size);
    }
};  // namespace mag
