#pragma once

#include "core/event.hpp"
#include "core/window.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"

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

            // @TODO: temp?
            void bind_buffers(Model* model);
            vk::DescriptorSet& get_material_descriptor(Material* material);
            // @TODO: temp?

            void add_model(Model* model);
            void remove_model(Model* model);

            void add_material(Material* material);
            void remove_material(Material* material);

        private:
            void on_resize(WindowResizeEvent& e);

            Window& window;
            std::unique_ptr<Context> context;

            std::map<Model*, VertexBuffer> vertex_buffers;
            std::map<Model*, IndexBuffer> index_buffers;

            // @TODO: create one per frame in flight if materials should change between frames
            std::map<Material*, vk::DescriptorSet> descriptor_sets;
            std::map<Material*, vk::DescriptorSetLayout> descriptor_set_layouts;
    };
};  // namespace mag
