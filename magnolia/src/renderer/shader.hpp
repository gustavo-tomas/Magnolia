#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "spirv_reflect.h"

namespace mag
{
    class Shader
    {
        public:
            void initialize(const str& file);
            void shutdown();

            const str& get_file() const { return file; };
            const vk::ShaderModule& get_handle() const { return module; };
            const SpvReflectShaderModule get_reflection() const { return spv_module; };

        private:
            vk::ShaderModule module = {};
            SpvReflectShaderModule spv_module = {};
            str file = {};
    };
};  // namespace mag
