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

    class PhysicsPass : public RenderGraphPass
    {
        public:
            PhysicsPass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            std::shared_ptr<Shader> physics_line_shader;
            std::unique_ptr<Line> physics_debug_lines;
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
