#pragma once

#include "core/window.hpp"
#include "renderer/context.hpp"

namespace mag
{
    class Renderer
    {
        public:
            Renderer(Window& window);
            ~Renderer();

            void begin();
            void end(const Image& image, const uvec3& image_size);

            void on_resize(const uvec2& size);

        private:
            Window& window;
            std::unique_ptr<Context> context;
    };
};  // namespace mag
