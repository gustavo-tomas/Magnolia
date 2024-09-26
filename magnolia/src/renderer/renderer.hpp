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
    struct TextComponent;
    struct QuadVertex;

    class Renderer
    {
        public:
            Renderer(Window& window);

            void update(RenderGraph& render_graph);
            void on_event(Event& e);

            void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                      const u32 first_instance = 0);

            void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                              const i32 vertex_offset = 0, const u32 first_instance = 0);

            // @TODO: temp?
            void bind_buffers(Model* model);
            void bind_buffers(TextComponent* text_c);
            ref<RendererImage> get_renderer_image(Image* image);
            // @TODO: temp?

            void upload_model(Model* model);
            void remove_model(Model* model);
            void update_model(Model* model);

            ref<RendererImage> upload_image(Image* image);
            void remove_image(Image* image);
            void update_image(Image* image);

            void upload_text(TextComponent* text_c, const std::vector<QuadVertex>& vertices,
                             const std::vector<u32>& indices);

            void remove_text(TextComponent* text_c);

        private:
            void on_resize(WindowResizeEvent& e);

            Window& window;
            unique<Context> context;

            // Buffer data
            std::map<void*, ref<VertexBuffer>> vertex_buffers;
            std::map<void*, ref<IndexBuffer>> index_buffers;

            // Image data
            std::map<Image*, ref<RendererImage>> images;
    };
};  // namespace mag
