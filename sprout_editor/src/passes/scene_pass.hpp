#pragma once

#include "renderer/render_graph.hpp"
#include "renderer/shader.hpp"

namespace sprout
{
    using namespace mag;

    class ScenePass : public RenderGraphPass
    {
        public:
            ScenePass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;

        private:
            ref<Shader> mesh_shader;
            ref<Shader> sprite_shader;
    };
};  // namespace sprout
