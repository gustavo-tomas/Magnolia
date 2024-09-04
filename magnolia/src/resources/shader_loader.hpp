#pragma once

#include "core/types.hpp"

namespace mag
{
    struct ShaderConfiguration;
    struct ShaderModule;

    class ShaderLoader
    {
        public:
            b8 load(const str& file_path, ShaderConfiguration* shader);

        private:
            b8 load_module(const str& file_path, ShaderModule* shader_module);
    };
};  // namespace mag
