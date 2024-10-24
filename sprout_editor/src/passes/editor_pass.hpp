#pragma once

#include "core/types.hpp"
#include "renderer/render_graph.hpp"

namespace mag
{
    struct TransformComponent;
    struct Image;

    class Shader;
    class Line;
};  // namespace mag

namespace sprout
{
    using namespace mag;

    class EditorPass : public RenderGraphPass
    {
        public:
            EditorPass(const uvec2& size);
            ~EditorPass();

            virtual void on_render(RenderGraph& render_graph) override;
    };

    class GizmoPass : public RenderGraphPass
    {
        public:
            GizmoPass(const uvec2& size);
            ~GizmoPass();

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            void render_sprites();
            void render_sprite(TransformComponent* transform, const ref<Image>& sprite, const u32 instance);
            void render_lines();
            void render_grid();

            ref<Shader> line_shader, grid_shader, sprite_shader;
            ref<Image> camera_sprite, light_sprite;
            unique<Line> lines;
    };
};  // namespace sprout
