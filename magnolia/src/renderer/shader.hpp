#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "spirv_reflect.h"

namespace mag
{
    using namespace mag::math;

    // @TODO: temporary
    struct alignas(16) LightData
    {
            vec3 color;     // 12 bytes (3 x 4)
            f32 intensity;  // 4 bytes  (1 x 4)
            vec3 position;  // 12 bytes (3 x 4)
    };
    // @TODO: temporary

    class ShaderModule
    {
        public:
            ShaderModule(const str& file, const vk::ShaderModule& module, const SpvReflectShaderModule& reflection)
                : module(module), spv_module(reflection), file(file)
            {
            }

            ~ShaderModule() = default;

            const str& get_file() const { return file; };
            const vk::ShaderModule& get_handle() const { return module; };
            const SpvReflectShaderModule& get_reflection() const { return spv_module; };

        private:
            vk::ShaderModule module = {};
            SpvReflectShaderModule spv_module = {};

            str file = {};
    };

    class Shader
    {
        public:
            Shader(const std::vector<std::shared_ptr<ShaderModule>>& modules);
            ~Shader() = default;

            struct UBO
            {
                    SpvReflectDescriptorBinding descriptor_binding;
                    std::vector<u8> data;
            };

            void add_attribute(const vk::Format format, const u32 size, const u32 offset);

            void set_uniform_global(const str& name, const void* data);
            void set_uniform_instance(const str& name, const void* data, const u32 instance);

            // @TODO: this is temporary. We want to avoid needing to pass modules around
            const std::vector<std::shared_ptr<ShaderModule>>& get_modules() const { return modules; };

            const std::vector<vk::VertexInputBindingDescription>& get_vertex_bindings() const
            {
                return vertex_bindings;
            };

            const std::vector<vk::VertexInputAttributeDescription>& get_vertex_attributes() const
            {
                return vertex_attributes;
            };

            const std::map<str, UBO>& get_uniforms() const { return uniforms_map; };

        private:
            void set_uniform(const str& scope, const str& name, const void* data, const u32 buffer_index);

            std::vector<std::shared_ptr<ShaderModule>> modules;
            std::vector<vk::VertexInputBindingDescription> vertex_bindings = {};
            std::vector<vk::VertexInputAttributeDescription> vertex_attributes = {};

            u32 location = 0;
            u32 stride = 0;
            std::map<str, UBO> uniforms_map;
    };

    class ShaderManager
    {
        public:
            ShaderManager() = default;
            ~ShaderManager();

            std::shared_ptr<Shader> load(const str& name, const str& vertex_file, const str& fragment_file);

        private:
            std::shared_ptr<ShaderModule> load_module(const str& file);
            std::map<str, std::shared_ptr<ShaderModule>> shader_modules;
            std::map<str, std::shared_ptr<Shader>> shaders;
    };
};  // namespace mag
