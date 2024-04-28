#pragma once

#include "camera/controller.hpp"
#include "core/window.hpp"
#include "editor/editor.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"

namespace mag
{
    // Expand app options if necessary
    typedef WindowOptions ApplicationOptions;

    class Application
    {
        public:
            explicit Application(const ApplicationOptions& options);
            virtual ~Application();

            void run();

            Window& get_window() { return window; };
            Editor& get_editor() { return editor; };
            ModelLoader& get_model_loader() { return model_loader; };
            TextureLoader& get_texture_loader() { return texture_loader; };
            Scene& get_active_scene() { return scene; };

        private:
            Window window;
            Renderer renderer;
            Editor editor;

            ModelLoader model_loader;
            TextureLoader texture_loader;

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
