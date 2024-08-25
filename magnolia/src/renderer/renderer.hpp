#pragma once

#include "core/event.hpp"
#include "core/window.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"

namespace mag
{
    struct Model;

    class Renderer
    {
        public:
            Renderer(Window& window);
            ~Renderer();

            void update(RenderGraph& render_graph);
            void on_event(Event& e);

            // @TODO: temp?
            void bind_buffers(Model* model);

            void add_model(Model* model);
            void remove_model(Model* model);

        private:
            void on_resize(WindowResizeEvent& e);

            Window& window;
            std::unique_ptr<Context> context;

            std::map<Model*, VertexBuffer> vertex_buffers;
            std::map<Model*, IndexBuffer> index_buffers;
    };
};  // namespace mag
