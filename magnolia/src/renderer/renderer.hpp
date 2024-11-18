#pragma once

#include "core/types.hpp"

// @TODO: refactor this API to be more consistent

namespace mag
{
    class Window;
    class RenderGraph;
    class Line;
    class Skydome;
    class RendererImage;

    struct Event;
    struct WindowResizeEvent;
    struct Model;
    struct Image;

    class Renderer
    {
        public:
            Renderer(Window& window);
            ~Renderer();

            void on_update(RenderGraph& render_graph);
            void on_event(Event& e);

            void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                      const u32 first_instance = 0);

            void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                              const i32 vertex_offset = 0, const u32 first_instance = 0);

            // @TODO: temp?
            void bind_buffers(Model* model);
            void bind_buffers(Line* line);
            void bind_buffers(Skydome* skydome);
            ref<RendererImage> get_renderer_image(Image* image);
            // @TODO: temp?

            void upload_model(Model* model);
            void remove_model(Model* model);
            void update_model(Model* model);

            ref<RendererImage> upload_image(Image* image);
            void remove_image(Image* image);
            void update_image(Image* image);

        private:
            void on_resize(WindowResizeEvent& e);

            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
