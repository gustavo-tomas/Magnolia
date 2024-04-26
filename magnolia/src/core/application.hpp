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

            enum class Mode
            {
                Editor = 0,
                Runtime
            };

            void run();

            Window& get_window() { return window; };
            Editor& get_editor() { return editor; };
            ModelLoader& get_model_loader() { return model_loader; };
            TextureLoader& get_texture_loader() { return texture_loader; };
            Scene& get_active_scene() { return scene; };
            const Mode& get_active_mode() const { return active_mode; };

            // @TODO: temp
            void set_active_mode(const Mode mode) { update_active_mode = mode; };

        private:
            Window window;
            Renderer renderer;
            Editor editor;
            Mode active_mode = Mode::Editor;
            Mode update_active_mode = Mode::Editor;

            ModelLoader model_loader;
            TextureLoader texture_loader;

            Scene scene;

            // @TODO: temp
            Cube cube;
            RuntimeController runtime_controller;
            EditorController editor_controller;
    };

    // @TODO: idk if this is thread safe but i wont use singletons <:(
    Application& get_application();

    // Defined by the client
    Application* create_application();
};  // namespace mag
