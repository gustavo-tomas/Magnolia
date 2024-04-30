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
        editor = std::make_unique<Editor>();
        LOG_SUCCESS("Editor initialized");

        // @TODO: temp create a basic scene
        scene.initialize();
        LOG_SUCCESS("Scene initialized");

        // Create a camera controller for editor
        editor_controller.initialize(&scene.get_camera());
        LOG_SUCCESS("EditorController initialized");

        // Set editor viewport image
        editor->set_viewport_image(scene.get_render_pass().get_target_image());

        // Set editor callbacks
        editor->on_viewport_resize(
            [&](const uvec2& size) mutable
            {
                LOG_INFO("VIEWPORT WINDOW RESIZE: {0}", math::to_string(size));
                scene.get_render_pass().on_resize(size);
                editor->set_viewport_image(scene.get_render_pass().get_target_image());
                scene.get_camera().set_aspect_ratio(size);
            });

        this->scene.get_render_pass().set_camera();

        // @TODO: temp load assets
        cube.initialize();

        cube.get_model().translation = vec3(0, 10, 0);
        cube.get_model().scale = vec3(10);

        scene.get_models().push_back(cube.get_model());
        scene.get_render_pass().add_model(cube.get_model());

        running = true;
    }

    Application::~Application()
    {
        cube.shutdown();

        this->editor_controller.shutdown();
        LOG_SUCCESS("EditorController destroyed");

        this->scene.shutdown();
        LOG_SUCCESS("Scene destroyed");
    }

    void Application::run()
    {
        u64 last_time = 0, curr_time = SDL_GetPerformanceCounter(), frame_counter = 0;
        f64 time_counter = 0.0, dt = 0.0;

        while (running)
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

            window->update();

            // Skip rendering if minimized
            if (window->is_minimized())
            {
                // @TODO: sleep for a lil bit
                continue;
            }

            editor_controller.update(dt);

            scene.update(dt);

            editor->update();

            renderer->update(scene.get_camera(), *editor, scene.get_render_pass(), scene.get_models());
        }
    }

    void Application::on_event(Event& e)
    {
        if (dynamic_cast<WindowCloseEvent*>(&e))
        {
            on_window_close(e);
        }

        else if (dynamic_cast<WindowResizeEvent*>(&e))
        {
            on_window_resize(e);
        }

        // @TODO: create on_event method to pipe these down

        else if (dynamic_cast<KeyPressEvent*>(&e))
        {
            on_key_press(e);
        }

        else if (dynamic_cast<MouseMoveEvent*>(&e))
        {
            on_mouse_move(e);
        }

        else if (dynamic_cast<MouseScrollEvent*>(&e))
        {
            on_mouse_scroll(e);
        }

        else if (dynamic_cast<SDLEvent*>(&e))
        {
            on_sdl_event(e);
        }
    }

    void Application::on_window_close(Event& e)
    {
        (void)e;
        running = false;
    }

    void Application::on_window_resize(Event& e)
    {
        const WindowResizeEvent* event = reinterpret_cast<WindowResizeEvent*>(&e);
        const uvec2 size = {event->width, event->height};

        renderer->on_resize(size);
        editor->on_resize(size);
    }

    void Application::on_key_press(Event& e)
    {
        const KeyPressEvent* event = reinterpret_cast<KeyPressEvent*>(&e);

        editor->on_key_press(event->key);
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

    void Application::on_sdl_event(Event& e)
    {
        SDLEvent* event = reinterpret_cast<SDLEvent*>(&e);

        this->editor->process_events(event->e);
    }
};  // namespace mag
