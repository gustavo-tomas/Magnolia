#pragma once

#include "core/types.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    class Scene;
    class RenderGraph;
    class Line;
    class RendererImage;

    struct Event;
    struct Model;
    struct Image;

    namespace gfx
    {
        b8 initialize();
        void shutdown();

        void on_update(RenderGraph& render_graph, Scene& scene);
        void on_event(const Event& e);

        void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                  const u32 first_instance = 0);

        void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                          const i32 vertex_offset = 0, const u32 first_instance = 0);

        // @TODO: temp?
        void bind_buffers(const Model* model);
        void bind_buffers(const Line* line);
        ref<RendererImage> get_renderer_image(const Image* image);
        // @TODO: temp?

        void upload_model(const Model* model);
        void remove_model(const Model* model);
        void update_model(const Model* model);

        ref<RendererImage> upload_image(const Image* image,
                                        const SamplerAddressMode address_mode = SamplerAddressMode::Repeat,
                                        const Filter min_mag_filter = Filter::Linear,
                                        const SamplerMipmapMode mip_map_mode = SamplerMipmapMode::Linear);
        void remove_image(const Image* image);
        void update_image(const Image* image);
    };  // namespace gfx
};      // namespace mag
