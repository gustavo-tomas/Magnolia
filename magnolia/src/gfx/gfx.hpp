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

        b8 initialize();
        void shutdown();

        void begin_frame();
        void end_frame();

        BufferHandle create_buffer(const u64 size, const void* data);
        void set_buffer_data(const BufferHandle buffer_handle, const u64 size, const void* data);

        ShaderHandle create_shader(const ShaderResource& shader);
        void use_shader(const ShaderHandle& handle);
        void set_shader_uniform(const ShaderHandle shader_handle, const BufferHandle buffer_handle,
                                const u32 binding = 0, const u32 array_element = 0);

        void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                  const u32 first_instance = 0);
    };  // namespace gfx
};      // namespace mag
