#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    struct ShaderResource;

    namespace gfx
    {
        // Keep it simple
        typedef u32 ShaderHandle;

        b8 initialize();

        void shutdown();

        void begin_frame();
        void end_frame();

        ShaderHandle create_shader(const ShaderResource& shader);
        void use_shader(const ShaderHandle& handle);

        void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                  const u32 first_instance = 0);
    };  // namespace gfx
};      // namespace mag
