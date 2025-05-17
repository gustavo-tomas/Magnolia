#pragma once

#include "core/types.hpp"

namespace mag
{
    struct ShaderResource;

    namespace gfx
    {
        // Keep it simple
        typedef u32 ShaderHandle;
        typedef u32 BufferHandle;
        typedef u32 TextureHandle;

        b8 initialize();
        void shutdown();

        void begin_frame();
        void end_frame();

        BufferHandle create_buffer(const u64 size, const void* data = nullptr);
        void set_buffer_data(const BufferHandle buffer_handle, const void* data, const u64 size, const u64 offset = 0);

        TextureHandle create_texture(const u32 width, const u32 height, const u64 size = 0,
                                     const void* pixels = nullptr);
        void set_texture_data(const TextureHandle texture_handle, const u64 size, const void* data);

        ShaderHandle create_shader(const ShaderResource& shader);
        void use_shader(const ShaderHandle& handle);
        void set_shader_buffer_uniform(const ShaderHandle shader_handle, const BufferHandle buffer_handle,
                                       const u32 binding = 0, const u32 array_element = 0);
        void set_shader_texture_uniform(const ShaderHandle shader_handle, const TextureHandle texture_handle,
                                        const u32 binding = 0, const u32 array_element = 0);

        void bind_vertex_buffer(const BufferHandle buffer_handle);
        void bind_index_buffer(const BufferHandle buffer_handle);

        void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                  const u32 first_instance = 0);

        void draw_indexed(const u32 index_count, const u32 instance_count = 1, const u32 first_index = 0,
                          const i32 vertex_offset = 0, const u32 first_instance = 0);
    };  // namespace gfx
};      // namespace mag
