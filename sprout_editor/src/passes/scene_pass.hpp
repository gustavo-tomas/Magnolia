#pragma once

#include "renderer/render_graph.hpp"
#include "renderer/shader.hpp"
#include "renderer/test_model.hpp"

namespace sprout
{
    using namespace mag;

    class ScenePass : public RenderGraphPass
    {
        public:
            ScenePass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            void draw_string(const f64 LineSpacing, const f64 Kerning);

            ref<Shader> mesh_shader;
            ref<Shader> sprite_shader;
            ref<Shader> text_shader;
    };

    class LinePass : public RenderGraphPass
    {
        public:
            LinePass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            ref<Shader> line_shader;
            unique<Line> lines;
    };

    class GridPass : public RenderGraphPass
    {
        public:
            GridPass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            ref<Shader> grid_shader;
    };
};  // namespace sprout
