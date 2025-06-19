#pragma once

#include <magnolia/core/event.hpp>
#include <magnolia/gfx/types.hpp>
#include <magnolia/scene/scene.hpp>

// This is the game renderer. It sits one layer above the gfx frontend and manages shaders, buffers and textures.

namespace game
{
    class Renderer
    {
        public:
            Renderer();
            ~Renderer();

            void on_event(const mag::Event& e);

            void render_scene(mag::Scene& scene, const f32 dt);

        private:
            void render_models(mag::Scene& scene);
            void render_sprites(mag::Scene& scene);
            void render_text(mag::Scene& scene);
            void render_debug(mag::Scene& scene, const f32 dt);

            mag::gfx::ShaderHandle sprite_shader;
            mag::gfx::ShaderHandle mesh_shader;
            mag::gfx::ShaderHandle text_shader;
            mag::gfx::ShaderHandle floor_shader;
            mag::gfx::ShaderHandle line_shader;
            mag::gfx::ShaderHandle debug_text_shader;
    };
};  // namespace game
