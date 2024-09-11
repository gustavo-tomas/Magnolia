#pragma once

#include <memory>

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
            std::shared_ptr<Shader> mesh_shader;
            std::shared_ptr<Shader> sprite_shader;
    };

    class LinePass : public RenderGraphPass
    {
        public:
            LinePass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

            // @TODO: make this buttons in the editor
            inline static b8 enable_bounding_boxes = true;
            inline static b8 enable_physics_boxes = true;

        private:
            std::shared_ptr<Shader> line_shader;
            std::unique_ptr<Line> lines;
    };

    class GridPass : public RenderGraphPass
    {
        public:
            GridPass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            std::shared_ptr<Shader> grid_shader;
    };
};  // namespace sprout
