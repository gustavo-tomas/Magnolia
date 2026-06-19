#pragma once

#include <magnolia/gfx/types.hpp>
#include <unordered_map>

// This is the game renderer. It sits one layer above the gfx frontend and manages shaders, buffers and textures.

namespace mag
{
    struct Event;
    struct ModelResource;
    struct TextureResource;
    struct FontResource;
};  // namespace mag

namespace game
{
    class Scene;

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
            void render_grass(Scene& scene);
            void render_text(Scene& scene);
            void set_grass_uniforms();

            b8 grass_uniforms_need_update = true;

            std::unordered_map<str, mag::gfx::ShaderHandle> shaders;
            std::unordered_map<str, mag::gfx::TextureHandle> texture_handles;
            std::unordered_map<str, mag::gfx::VertexBufferHandle> vertex_buffer_handles;
            std::unordered_map<str, mag::gfx::IndexBufferHandle> index_buffer_handles;

            struct FontData
            {
                    std::unordered_map<c8, mag::gfx::TextureHandle> char_texture_handles;
                    u32 idx = 0;
            };

            std::unordered_map<str, FontData> fonts;
    };
};  // namespace game
