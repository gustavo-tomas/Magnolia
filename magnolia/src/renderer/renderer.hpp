#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "core/window.hpp"
#include "editor/editor.hpp"
#include "renderer/context.hpp"
#include "renderer/model.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    class Renderer
    {
        public:
            void initialize(Window& window);
            void shutdown();

            void update(Editor& editor, StandardRenderPass& render_pass, std::vector<Model>& models, const f32 dt);

            void on_resize(const uvec2& size);
            void on_mouse_move(const ivec2& mouse_dir);

        private:
            Window* window;
            Context context;
            Camera camera;
            Controller controller;
    };
};  // namespace mag
