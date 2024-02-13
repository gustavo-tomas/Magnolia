#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "spirv_reflect.h"

namespace mag
{
    class Shader
    {
        public:
            struct SpvReflection
            {
                    u32 binding = {};
                    vk::DescriptorType descriptor_type = {};
                    vk::ShaderStageFlagBits shader_stage = {};
            };

            void initialize(const str& file);
            void shutdown();

            const str& get_file() const { return file; };
            const vk::ShaderModule& get_handle() const { return module; };
            const SpvReflection get_reflection() const { return spv_reflection; };

        private:
            vk::ShaderModule module = {};
            SpvReflectShaderModule spv_module = {};
            SpvReflection spv_reflection = {};
            str file = {};
    };
};  // namespace mag
