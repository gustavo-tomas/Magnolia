#pragma once

#include <gfx/types.hpp>
#include <magnolia.hpp>
#include <scene/scene.hpp>

// This is the game renderer. It sits one layer above the gfx frontend and manages shaders, buffers and textures.

namespace game
{
    class Renderer
    {
        public:
            Renderer();
            ~Renderer();

            void on_event(const mag::Event& e);

            void render_scene(mag::Scene& scene);

        private:
            void render_models(mag::Scene& scene);
            void render_sprites(mag::Scene& scene);
            void render_text(mag::Scene& scene);

            mag::gfx::ShaderHandle sprite_shader;
            mag::gfx::ShaderHandle mesh_shader;
            mag::gfx::ShaderHandle text_shader;
    };
};  // namespace game
