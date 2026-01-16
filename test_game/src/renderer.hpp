#pragma once

#include <magnolia/core/event.hpp>
#include <magnolia/gfx/types.hpp>

#include "scene.hpp"

namespace game
{
    class Renderer
    {
        public:
            Renderer();
            ~Renderer();

            void on_event(const mag::Event& e);

            void render_scene(Scene& scene, const f32 dt);

        private:
            void render_debug(Scene& scene, const f32 dt);

            mag::gfx::ShaderHandle debug_text_shader;
    };
};  // namespace game
