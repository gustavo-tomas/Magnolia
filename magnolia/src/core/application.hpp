#pragma once

#include <memory>

#include "camera/controller.hpp"
#include "core/event.hpp"
#include "core/window.hpp"
#include "editor/editor.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"

namespace mag
{
    // Expand app options if necessary
    struct ApplicationOptions
    {
            uvec2 size = WindowOptions::MAX_SIZE;
            ivec2 position = WindowOptions::CENTER_POS;
            str title = "Magnolia";
    };

    class Application
    {
        public:
            explicit Application(const ApplicationOptions& options);
            virtual ~Application();

            void run();
            void on_event(Event& e);

            Window& get_window() { return *window; };
            Editor& get_editor() { return *editor; };
            ModelLoader& get_model_loader() { return *model_loader; };
            TextureLoader& get_texture_loader() { return *texture_loader; };
            Scene& get_active_scene() { return scene; };

        private:
            void on_window_close(Event& e);
            void on_window_resize(Event& e);

            std::unique_ptr<Window> window;
            std::unique_ptr<Renderer> renderer;
            std::unique_ptr<Editor> editor;
            std::unique_ptr<ModelLoader> model_loader;
            std::unique_ptr<TextureLoader> texture_loader;

            b8 running;

            EventManager event_manager;
            Scene scene;

            // @TODO: temp
            Cube cube;

            // @TODO: idk what im doing so we'll keep things simple with only the editor controller
            EditorController editor_controller;
    };

    // @TODO: idk if this is thread safe but i wont use singletons <:(
    Application& get_application();

    // Defined by the client
    Application* create_application();
};  // namespace mag
