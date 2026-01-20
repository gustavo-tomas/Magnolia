#pragma once

#include <magnolia/core/event.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/gfx/types.hpp>
#include <unordered_map>

#include "scene.hpp"

// This is the game renderer. It sits one layer above the gfx frontend and manages shaders, buffers and textures.

namespace game
{
    class Renderer
    {
        public:
            Renderer();
            ~Renderer();

            void on_event(const mag::Event& e);

            void render_scene(Scene& scene, const f32 dt);

            void build_shader(const str& file_path, const b8 recompile = false);

        private:
            void render_models(Scene& scene);
            void render_sprites(Scene& scene);
            void render_text(Scene& scene);
            void render_debug(Scene& scene, const f32 dt);

            std::unordered_map<str, mag::gfx::ShaderHandle> shaders;
    };
};  // namespace game
