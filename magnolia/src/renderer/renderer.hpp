#pragma once

#include "core/event.hpp"
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

            void on_event(Event& e);

        private:
            void on_resize(WindowResizeEvent& e);

            Window& window;
            std::unique_ptr<Context> context;
    };
};  // namespace mag
