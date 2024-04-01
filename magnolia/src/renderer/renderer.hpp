#pragma once

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

            void update(const Camera& camera, Editor& editor, StandardRenderPass& render_pass,
                        std::vector<Model>& models);

            void on_resize(const uvec2& size);

        private:
            Window* window;
            Context context;
    };
};  // namespace mag
