#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"

namespace mag
{
    struct ShaderResource;
    struct Event;

    namespace gfx
    {
        MAG_API b8 initialize(const GfxOptions& options);

        MAG_API void shutdown();

        MAG_API b8 begin_frame();

        MAG_API b8 end_frame();

        MAG_API VertexBufferHandle create_vertex_buffer(u64 size, const void* data = nullptr);

        MAG_API void destroy_vertex_buffer(VertexBufferHandle vertex_buffer_handle);

        MAG_API IndexBufferHandle create_index_buffer(u64 size, const void* data = nullptr);

        MAG_API TextureHandle create_texture(u32 width, u32 height, u64 size = 0, const void* pixels = nullptr,
                                             Format format = Format::R8G8B8A8_SRGB);

        MAG_API ShaderHandle create_shader(const ShaderResource& shader);

        MAG_API void destroy_shader(ShaderHandle shader_handle);

        // Bind the shader before setting the uniforms
        MAG_API void use_shader(const ShaderHandle& handle);

        // The uniforms are set according to the last bound shader
        MAG_API void set_uniform(const str& uniform_name, const void* data, u32 array_element = 0);

        MAG_API void set_uniform(const str& uniform_name, TextureHandle texture_handle, u32 array_element = 0);

        // Set a uniform for every frame-in-flight. Useful for data that doesn't change between frames like static
        // objects.
        MAG_API void set_uniform_static(const str& uniform_name, const void* data, u32 array_element = 0);

        MAG_API void set_uniform_static(const str& uniform_name, TextureHandle texture_handle, u32 array_element = 0);

        MAG_API void bind_vertex_buffer(VertexBufferHandle vertex_buffer_handle);

        MAG_API void bind_index_buffer(IndexBufferHandle index_buffer_handle);

        MAG_API void draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0);

        MAG_API void draw_indexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, i32 vertex_offset = 0,
                                  u32 first_instance = 0);

        MAG_API void on_event(const Event& e);
    };  // namespace gfx
};  // namespace mag
