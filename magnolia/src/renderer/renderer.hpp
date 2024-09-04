#pragma once

#include "core/event.hpp"
#include "core/window.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"
#include "resources/image.hpp"

// @TODO: refactor this API to be more consistent

namespace mag
{
    struct Model;
    struct Material;

    class Renderer
    {
        public:
            Renderer(Window& window);
            ~Renderer();

            void update(RenderGraph& render_graph);
            void on_event(Event& e);

            void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                      const u32 first_instance = 0);

            void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                              const i32 vertex_offset = 0, const u32 first_instance = 0);

            // @TODO: temp?
            void bind_buffers(Model* model);
            std::shared_ptr<RendererImage> get_renderer_image(Image* image);
            // @TODO: temp?

            void upload_model(Model* model);
            void remove_model(Model* model);
            void update_model(Model* model);

            std::shared_ptr<RendererImage> upload_image(Image* image);
            void remove_image(Image* image);
            void update_image(Image* image);

        private:
            void on_resize(WindowResizeEvent& e);

            Window& window;
            std::unique_ptr<Context> context;

            // Model data
            std::map<Model*, VertexBuffer> vertex_buffers;
            std::map<Model*, IndexBuffer> index_buffers;

            // Image data
            std::map<Image*, std::shared_ptr<RendererImage>> images;
    };
};  // namespace mag
