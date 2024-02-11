#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "spirv_reflect.h"

namespace mag
{
    class Shader
    {
        public:
            void initialize(const str& file, const vk::ShaderStageFlagBits stage);
            void shutdown();

            const str& get_file() const { return file; };
            const vk::ShaderModule& get_handle() const { return module; };
            vk::ShaderStageFlagBits get_stage() const { return stage; };

        private:
            vk::ShaderModule module = {};
            vk::ShaderStageFlagBits stage = {};
            SpvReflectShaderModule spv_module = {};
            str file = {};
    };
};  // namespace mag
