#pragma once

#include "core/window.hpp"
#include "editor/editor.hpp"
#include "renderer/context.hpp"
#include "scene/scene.hpp"

namespace mag
{
    class Renderer
    {
        public:
            Renderer(Window& window);
            ~Renderer();

            void update(BaseScene& scene, Editor& editor);

            void on_resize(const uvec2& size);

            const Statistics& get_statistics() const { return statistics; };

        private:
            Window& window;
            std::unique_ptr<Context> context;
            Statistics statistics;
    };
};  // namespace mag
