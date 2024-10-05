#pragma once

#include "ecs/components.hpp"
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
            void draw_text(TextComponent* text_c, const mat4& transform);

            ref<Shader> mesh_shader;
            ref<Shader> sprite_shader;
            ref<Shader> text_shader;
    };
};  // namespace sprout
