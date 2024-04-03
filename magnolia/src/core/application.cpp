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

    void Application::initialize(const str& title, const uvec2& size)
    {
        application = this;

        WindowOptions window_options;
        window_options.size = size;
        window_options.title = title;

        // Create the window
        window.initialize(window_options);
        LOG_SUCCESS("Window initialized");

        // Create the renderer
        renderer.initialize(window);
        LOG_SUCCESS("Renderer initialized");

        // Create the editor
        editor.initialize(window);
        LOG_SUCCESS("Editor initialized");

        // Create the model loader
        model_loader.initialize();
        LOG_SUCCESS("ModelLoader initialized");

        // Create the texture loader
        texture_loader.initialize();
        LOG_SUCCESS("TextureLoader initialized");

        // Create a render pass
        render_pass.initialize(window.get_size());
        LOG_SUCCESS("RenderPass initialized");

        // Create a camera
        camera.initialize({-100.0f, 5.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, window.get_size(), 0.1f, 10000.0f);
        LOG_SUCCESS("Camera initialized");

        // Create a camera controller
        controller.initialize(&camera);
        LOG_SUCCESS("Controller initialized");

        // Set window callbacks
        window.on_resize(
            [&](const uvec2& size) mutable
            {
                LOG_INFO("WINDOW RESIZE: {0}", math::to_string(size));
                camera.set_aspect_ratio(size);
                renderer.on_resize(size);
                editor.on_resize(size);
                render_pass.on_resize(size);
            });

        window.on_key_press([](const SDL_Keycode key) mutable { LOG_INFO("KEY PRESS: {0}", SDL_GetKeyName(key)); });
        window.on_key_release([](const SDL_Keycode key) mutable { LOG_INFO("KEY RELEASE: {0}", SDL_GetKeyName(key)); });
        window.on_mouse_move([this](const ivec2& mouse_dir) mutable { this->controller.on_mouse_move(mouse_dir); });
        window.on_button_press([](const u8 button) mutable { LOG_INFO("BUTTON PRESS: {0}", button); });
        window.on_event([this](SDL_Event e) mutable { this->editor.process_events(e); });

        // @TODO: temp load assets
        cube.initialize();

        cube.get_model().position = vec3(0, 10, 0);
        cube.get_model().scale = vec3(10);

        models.push_back(cube.get_model());
        models.push_back(*model_loader.load("assets/models/sponza/sponza.obj"));

        this->render_pass.set_camera();
        for (const auto& m : models) this->render_pass.add_model(m);
    }

    void Application::shutdown()
    {
        cube.shutdown();

        this->controller.shutdown();
        LOG_SUCCESS("Controller destroyed");

        this->camera.shutdown();
        LOG_SUCCESS("Camera destroyed");

        this->render_pass.shutdown();
        LOG_SUCCESS("RenderPass destroyed");

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

            if (window.is_key_pressed(SDLK_ESCAPE))
            {
                // Fullscreen
                if (!window.is_flag_set(SDL_WINDOW_FULLSCREEN_DESKTOP))
                    window.set_fullscreen(SDL_WINDOW_FULLSCREEN_DESKTOP);

                // Windowed
                else
                    window.set_fullscreen(0);
            }

            if (window.is_key_pressed(SDLK_TAB)) window.set_capture_mouse(!window.is_mouse_captured());

            controller.update(dt);

            // @TODO: testing
            models[0].rotation = models[0].rotation + vec3(0, 60.0f * dt, 0);

            for (u32 i = 0; i < 3; i++) models[0].rotation[i] = fmod(models[0].rotation[i], 360.0);

            if (window.is_key_down(SDLK_UP))
                render_pass.set_render_scale(render_pass.get_render_scale() + 0.15f * dt);

            else if (window.is_key_down(SDLK_DOWN))
                render_pass.set_render_scale(render_pass.get_render_scale() - 0.15f * dt);
            // @TODO: testing

            // Skip rendering if minimized
            if (!window.is_minimized()) renderer.update(camera, editor, render_pass, models);
        }
    }
};  // namespace mag
