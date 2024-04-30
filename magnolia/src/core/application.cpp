#include "core/application.hpp"

#include "core/event.hpp"
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

        // Create the window
        const WindowOptions window_options = {event_manager, options.size, options.position, options.title};
        window = std::make_unique<Window>(window_options);
        LOG_SUCCESS("Window initialized");

        // Create the renderer
        renderer.initialize(*window);
        LOG_SUCCESS("Renderer initialized");

        // Create the editor
        editor.initialize();
        LOG_SUCCESS("Editor initialized");

        // Create the model loader
        model_loader.initialize();
        LOG_SUCCESS("ModelLoader initialized");

        // Create the texture loader
        texture_loader.initialize();
        LOG_SUCCESS("TextureLoader initialized");

        // @TODO: temp create a basic scene
        scene.initialize();
        LOG_SUCCESS("Scene initialized");

        // Create a camera controller for editor
        editor_controller.initialize(&scene.get_camera());
        LOG_SUCCESS("EditorController initialized");

        // Set window callbacks
        event_manager.subscribe(EventType::WindowResize, BIND_FN(Application::on_window_resize));
        event_manager.subscribe(EventType::KeyPress, BIND_FN(Application::on_key_press));
        event_manager.subscribe(EventType::MouseMove, BIND_FN(Application::on_mouse_move));
        event_manager.subscribe(EventType::MouseScroll, BIND_FN(Application::on_mouse_scroll));
        event_manager.subscribe(EventType::SDLEvent, BIND_FN(Application::on_event));

        // Set editor viewport image
        editor.set_viewport_image(scene.get_render_pass().get_target_image());

        // Set editor callbacks
        editor.on_viewport_resize(
            [&](const uvec2& size) mutable
            {
                LOG_INFO("VIEWPORT WINDOW RESIZE: {0}", math::to_string(size));
                scene.get_render_pass().on_resize(size);
                editor.set_viewport_image(scene.get_render_pass().get_target_image());
                scene.get_camera().set_aspect_ratio(size);
            });

        this->scene.get_render_pass().set_camera();

        // @TODO: temp load assets
        cube.initialize();

        cube.get_model().translation = vec3(0, 10, 0);
        cube.get_model().scale = vec3(10);

        scene.get_models().push_back(cube.get_model());
        scene.get_render_pass().add_model(cube.get_model());
    }

    Application::~Application()
    {
        cube.shutdown();

        this->editor_controller.shutdown();
        LOG_SUCCESS("EditorController destroyed");

        this->scene.shutdown();
        LOG_SUCCESS("Scene destroyed");

        texture_loader.shutdown();
        LOG_SUCCESS("TextureLoader destroyed");

        model_loader.shutdown();
        LOG_SUCCESS("ModelLoader destroyed");

        editor.shutdown();
        LOG_SUCCESS("Editor destroyed");

        renderer.shutdown();
        LOG_SUCCESS("Renderer destroyed");
    }

    void Application::run()
    {
        u64 last_time = 0, curr_time = SDL_GetPerformanceCounter(), frame_counter = 0;
        f64 time_counter = 0.0, dt = 0.0;

        while (window->update())
        {
            // Calculate dt
            last_time = curr_time;
            curr_time = SDL_GetPerformanceCounter();
            dt = (curr_time - last_time) * 1000.0 / (SDL_GetPerformanceFrequency() * 1000.0);

            frame_counter++;
            time_counter += dt;
            if (time_counter >= 1.0)
            {
                LOG_INFO("CPU: {0:.3f} ms/frame - {1} fps", 1000.0 / static_cast<f64>(frame_counter), frame_counter);
                frame_counter = time_counter = 0;
            }

            // Skip rendering if minimized
            if (window->is_minimized())
            {
                // @TODO: sleep for a lil bit
                continue;
            }

            editor_controller.update(dt);

            scene.update(dt);

            editor.update();

            renderer.update(scene.get_camera(), editor, scene.get_render_pass(), scene.get_models());
        }
    }

    void Application::on_window_resize(Event& e)
    {
        const WindowResizeEvent* event = reinterpret_cast<WindowResizeEvent*>(&e);
        const uvec2 size = {event->width, event->height};

        renderer.on_resize(size);
        editor.on_resize(size);
    }

    void Application::on_key_press(Event& e)
    {
        const KeyPressEvent* event = reinterpret_cast<KeyPressEvent*>(&e);

        editor.on_key_press(event->key);
    }

    void Application::on_mouse_move(Event& e)
    {
        const MouseMoveEvent* event = reinterpret_cast<MouseMoveEvent*>(&e);
        const ivec2 mouse_dir = {event->x_direction, event->y_direction};

        this->editor_controller.on_mouse_move(mouse_dir);
    }

    void Application::on_mouse_scroll(Event& e)
    {
        const MouseScrollEvent* event = reinterpret_cast<MouseScrollEvent*>(&e);
        const vec2 offset = {event->x_offset, event->y_offset};

        this->editor_controller.on_wheel_move(offset);
    }

    void Application::on_event(Event& e)
    {
        SDLEvent* event = reinterpret_cast<SDLEvent*>(&e);

        this->editor.process_events(event->e);
    }
};  // namespace mag
