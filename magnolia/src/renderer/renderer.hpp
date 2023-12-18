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

        private:
            Context context;
    };
};  // namespace mag
