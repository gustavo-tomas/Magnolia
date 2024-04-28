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

        WindowOptions window_options;
        window_options.size = options.size;
        window_options.title = options.title;

        // Create the window
        window.initialize(window_options);
        LOG_SUCCESS("Window initialized");

        // Create the renderer
        renderer.initialize(window);
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
        window.on_resize(
            [&](const uvec2& size) mutable
            {
                LOG_INFO("WINDOW RESIZE: {0}", math::to_string(size));
                renderer.on_resize(size);
                editor.on_resize(size);

                // @TODO: change sizes in runtime!
                // render_pass.on_resize(size);
                // editor.set_viewport_image(render_pass.get_target_image());
                // camera.set_aspect_ratio(size);
            });

        window.on_key_press(
            [this](const SDL_Keycode key) mutable
            {
                LOG_INFO("KEY PRESS: {0}", SDL_GetKeyName(key));
                editor.on_key_press(key);
            });

        window.on_key_release([](const SDL_Keycode key) mutable { LOG_INFO("KEY RELEASE: {0}", SDL_GetKeyName(key)); });
        window.on_mouse_move([this](const ivec2& mouse_dir) mutable
                             { this->editor_controller.on_mouse_move(mouse_dir); });

        window.on_wheel_move([this](const ivec2& wheel_dir) mutable
                             { this->editor_controller.on_wheel_move(wheel_dir); });

        window.on_button_press([](const u8 button) mutable { LOG_INFO("BUTTON PRESS: {0}", button); });
        window.on_event([this](SDL_Event e) mutable { this->editor.process_events(e); });

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

        window.shutdown();
        LOG_SUCCESS("Window destroyed");
    }

    void Application::run()
    {
        u64 last_time = 0, curr_time = SDL_GetPerformanceCounter(), frame_counter = 0;
        f64 time_counter = 0.0, dt = 0.0;

        while (window.update())
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

            editor_controller.update(dt);

            scene.update(dt);

            editor.update();

            // Skip rendering if minimized
            if (!window.is_minimized())
                renderer.update(scene.get_camera(), editor, scene.get_render_pass(), scene.get_models());
        }
    }
};  // namespace mag
