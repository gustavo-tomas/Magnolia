#pragma once

#include "core/window.hpp"
#include "editor/editor.hpp"
#include "renderer/model.hpp"
#include "renderer/renderer.hpp"

namespace mag
{
    class Application
    {
        public:
            void initialize(const str& title, const u32 width = 800, const u32 height = 600);
            void shutdown();

            void run();

            static ModelLoader& get_model_loader() { return model_loader; };

        private:
            Window window;
            Renderer renderer;
            Editor editor;

            static ModelLoader model_loader;

            // @TODO: temp
            std::shared_ptr<Model> model;
            Cube cube;
    };
};  // namespace mag
