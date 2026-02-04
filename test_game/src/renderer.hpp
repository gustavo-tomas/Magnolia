#pragma once

#include <magnolia/core/event.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/gfx/types.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/texture.hpp>
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

            void on_model_added(const mag::ModelResource& model);

            void on_texture_added(const mag::TextureResource& texture);

            void on_font_added(const mag::FontResource& font);

            void render_scene(Scene& scene, const f32 dt);

            void build_shader(const str& file_path, const b8 recompile = false);

        private:
            void render_models(Scene& scene);
            void render_sprites(Scene& scene);
            void render_text(Scene& scene);

            std::unordered_map<str, mag::gfx::ShaderHandle> shaders;
            std::unordered_map<str, mag::gfx::TextureHandle> texture_handles;
            std::unordered_map<str, mag::gfx::VertexBufferHandle> vertex_buffer_handles;
            std::unordered_map<str, mag::gfx::IndexBufferHandle> index_buffer_handles;

            struct FontData
            {
                    std::unordered_map<c8, mag::gfx::TextureHandle> char_texture_handles;
                    u32 idx;
            };

            std::unordered_map<str, FontData> fonts;
    };
};  // namespace game
