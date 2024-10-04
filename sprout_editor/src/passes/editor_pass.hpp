#pragma once

#include "renderer/render_graph.hpp"
#include "renderer/shader.hpp"
#include "renderer/test_model.hpp"

namespace sprout
{
    using namespace mag;

    class EditorPass : public RenderGraphPass
    {
        public:
            EditorPass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;
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
