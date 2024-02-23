#pragma once

#include "renderer/image.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    class EditorRenderPass : public RenderPass
    {
        public:
            virtual void initialize(const uvec2& size) override;
            virtual void shutdown() override;

            virtual void before_render(CommandBuffer& command_buffer) override;
            virtual void after_render(CommandBuffer& command_buffer) override;

            virtual Pass& get_pass() override { return pass; };
            const Image& get_draw_image() const { return draw_image; };
            uvec3 get_draw_size() const { return draw_size; };

            void on_resize(const uvec2& size);

        private:
            void initialize_draw_image();

            Pass pass = {};
            Image draw_image;
            uvec3 draw_size;
    };
};  // namespace mag
