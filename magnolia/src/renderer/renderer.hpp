#pragma once

#include "core/window.hpp"
#include "renderer/context.hpp"

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
    };
};  // namespace mag
