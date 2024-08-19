#pragma once

#include "core/window.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"

namespace mag
{
    class Renderer
    {
        public:
            Renderer(Window& window);
            ~Renderer();

            void update(RenderGraph& render_graph);

            void on_resize(const uvec2& size);

        private:
            Window& window;
            std::unique_ptr<Context> context;
    };
};  // namespace mag
