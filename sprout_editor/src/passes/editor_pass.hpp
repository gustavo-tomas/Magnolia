#pragma once

#include "renderer/render_graph.hpp"

namespace sprout
{
    using namespace mag;

    class EditorPass : public RenderGraphPass
    {
        public:
            EditorPass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;
    };
};  // namespace sprout
