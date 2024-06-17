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
            const std::unique_ptr<Line>& get_physics_debug_lines() const { return physics_debug_lines; };

        private:
            Window& window;
            std::unique_ptr<Context> context;
            std::unique_ptr<Line> physics_debug_lines;
            Statistics statistics;
    };
};  // namespace mag
