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
            void run();

            static Application& get();

            Window& get_window() { return window; };
            ModelLoader& get_model_loader() { return model_loader; };
            TextureLoader& get_texture_loader() { return texture_loader; };

        private:
            Application();
            ~Application();

            static Application* instance;

            Window window;
            Renderer renderer;
            Editor editor;
            Controller controller;
            Camera camera;

            ModelLoader model_loader;
            TextureLoader texture_loader;

            // @TODO: temp
            StandardRenderPass render_pass;
            std::vector<Model> models;
            Cube cube;
    };
};  // namespace mag
