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

            void add_attribute(const vk::Format format, const u32 size, const u32 offset);

            const str& get_file() const { return file; };
            const vk::ShaderModule& get_handle() const { return module; };
            const vk::VertexInputBindingDescription& get_vertex_binding() const { return binding; };
            const std::vector<vk::VertexInputAttributeDescription>& get_vertex_attributes() const
            {
                return attributes;
            };
            const SpvReflectShaderModule& get_reflection() const { return spv_module; };

        private:
            vk::ShaderModule module = {};
            SpvReflectShaderModule spv_module = {};
            vk::VertexInputBindingDescription binding = {};
            std::vector<vk::VertexInputAttributeDescription> attributes = {};
            str file = {};
            u32 location = 0;
            u32 stride = 0;
    };
};  // namespace mag
