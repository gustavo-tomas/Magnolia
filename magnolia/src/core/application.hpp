#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "core/window.hpp"
#include "editor/editor.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/renderer.hpp"

namespace mag
{
    class Application
    {
        public:
            void initialize(const str& title, const uvec2& size = WindowOptions::MAX_SIZE);
            void shutdown();

            void run();

            Window& get_window() { return window; };
            Editor& get_editor() { return editor; };
            ModelLoader& get_model_loader() { return model_loader; };
            TextureLoader& get_texture_loader() { return texture_loader; };

            // @TODO: temp
            void add_model(const str& path);

        private:
            enum class Mode
            {
                Editor = 0,
                Runtime
            };

            Window window;
            Renderer renderer;
            Editor editor;
            Mode active_mode = Mode::Editor;

            ModelLoader model_loader;
            TextureLoader texture_loader;

            // @TODO: temp
            StandardRenderPass render_pass;
            std::vector<Model> models;
            std::vector<str> models_queue;
            Cube cube;
            Camera camera;
            RuntimeController runtime_controller;
            EditorController editor_controller;
    };

    // @TODO: idk if this is thread safe but i wont use singletons <:(
    Application& get_application();
};  // namespace mag
