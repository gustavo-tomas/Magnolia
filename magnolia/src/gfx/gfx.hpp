#pragma once

#include "core/types.hpp"
#include "gfx/types.hpp"

namespace mag
{
    struct ShaderResource;
    struct Event;

    namespace gfx
    {
        b8 initialize(const GfxOptions& options);

        void shutdown();

        b8 begin_frame();

        b8 end_frame();

        VertexBufferHandle create_vertex_buffer(const u64 size, const void* data = nullptr);

        IndexBufferHandle create_index_buffer(const u64 size, const void* data = nullptr);

        TextureHandle create_texture(const u32 width, const u32 height, const u64 size = 0,
                                     const void* pixels = nullptr, const Format format = Format::R8G8B8A8_SRGB);

        ShaderHandle create_shader(const ShaderResource& shader);

        // Bind the shader before setting the uniforms
        void use_shader(const ShaderHandle& handle);

        // The uniforms are set according to the last bound shader
        void set_uniform(const str& uniform_name, const void* data, const u32 array_element = 0);

        void set_uniform(const str& uniform_name, const TextureHandle texture_handle, const u32 array_element = 0);

        void bind_vertex_buffer(const VertexBufferHandle vertex_buffer_handle);

        void bind_index_buffer(const IndexBufferHandle index_buffer_handle);

        void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                  const u32 first_instance = 0);

        void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                          const i32 vertex_offset = 0, const u32 first_instance = 0);

        void on_event(const Event& e);
    };  // namespace gfx
};      // namespace mag
