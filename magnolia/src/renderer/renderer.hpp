#pragma once

#include "core/window.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    class Renderer
    {
        public:
            void initialize(Window& window);
            void shutdown();

            void update();

            void on_resize(const uvec2& size);

        private:
            Window* window;
            Context context;
            StandardRenderPass render_pass;
            Image draw_image;
            uvec2 draw_image_resolution = {800 / 2, 600 / 2};
    };
};  // namespace mag
