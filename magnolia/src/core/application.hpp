#pragma once

#include <future>
#include <mutex>

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
            void initialize(const str& title, const u32 width = 800, const u32 height = 600);
            void shutdown();

            void run();

            static ModelLoader& get_model_loader() { return model_loader; };
            static TextureLoader& get_texture_loader() { return texture_loader; };

            static void load_mesh(const str file);

        private:
            static Window window;
            Renderer renderer;
            Editor editor;

            static ModelLoader model_loader;
            static TextureLoader texture_loader;

            // @TODO: temp
            Cube cube;

            static StandardRenderPass render_pass;
            static std::mutex meshes_mutex;
            static std::vector<Model> models;
            static std::vector<std::future<void>> futures;
    };
};  // namespace mag
