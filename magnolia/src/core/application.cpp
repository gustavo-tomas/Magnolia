#include "core/application.hpp"

#include "core/logger.hpp"

namespace mag
{
    ModelLoader Application::model_loader;
    TextureLoader Application::texture_loader;

    void Application::initialize(const str& title, const u32 width, const u32 height)
    {
        WindowOptions window_options;
        window_options.size = {width, height};
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

        // Set window callbacks
        window.on_resize(
            [&](const uvec2& size) mutable
            {
                LOG_INFO("WINDOW RESIZE: {0}", math::to_string(size));
                renderer.on_resize(size);
                editor.on_resize(size);
                render_pass.on_resize(size);
            });

        window.on_key_press([](const SDL_Keycode key) mutable { LOG_INFO("KEY PRESS: {0}", SDL_GetKeyName(key)); });
        window.on_key_release([](const SDL_Keycode key) mutable { LOG_INFO("KEY RELEASE: {0}", SDL_GetKeyName(key)); });
        window.on_mouse_move([this](const ivec2& mouse_dir) mutable { this->renderer.on_mouse_move(mouse_dir); });
        window.on_button_press([](const u8 button) mutable { LOG_INFO("BUTTON PRESS: {0}", button); });
        window.on_event([this](SDL_Event e) mutable { this->editor.process_events(e); });

        // @TODO: temp load assets
        cube.initialize();

        auto& cube_model_matrix = cube.get_model().model_matrix;
        cube_model_matrix = translate(cube_model_matrix, vec3(0, 10, 0));
        cube_model_matrix = scale(cube_model_matrix, vec3(10.0f));
        models.push_back(cube.get_model());

        model = Application::get_model_loader().load("assets/models/sponza/sponza.obj");
        models.push_back(*model);

        this->render_pass.initialize(window.get_size(), models);
        LOG_SUCCESS("RenderPass initialized");
    }

    void Application::shutdown()
    {
        cube.shutdown();

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

            // @TODO: spiniignigngginign
            auto& cube_model_matrix = models[0].model_matrix;
            cube_model_matrix = rotate(cube_model_matrix, radians(60.0f * static_cast<f32>(dt)), vec3(0, 1, 0));

            renderer.update(editor, render_pass, models, dt);
        }
    }
};  // namespace mag
