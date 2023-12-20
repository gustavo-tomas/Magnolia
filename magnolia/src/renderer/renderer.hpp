#pragma once

#include "core/window.hpp"
#include "renderer/context.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    class Renderer
    {
        public:
            void initialize(Window& window);
            void shutdown();

            void update();

            void resize(const uvec2& size);

        private:
            Window* window;
            Context context;
            StandardRenderPass render_pass;
    };
};  // namespace mag
