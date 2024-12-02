#pragma once

#include "renderer/render_graph.hpp"

namespace mag
{
    class Shader;
};

namespace sprout
{
    using namespace mag;

    class DepthPrePass : public RenderGraphPass
    {
        public:
            DepthPrePass(const uvec2& size);
            ~DepthPrePass();

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            ref<Shader> depth_prepass_shader;
    };

    class ScenePass : public RenderGraphPass
    {
        public:
            ScenePass(const uvec2& size);
            ~ScenePass();

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            ref<Shader> mesh_shader;
            ref<Shader> sprite_shader;
    };
};  // namespace sprout
