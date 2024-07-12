#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/buffers.hpp"
#include "spirv_reflect.h"

namespace mag
{
    using namespace mag::math;

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

    class Pipeline;
    class Shader
    {
        public:
            Shader(const std::vector<std::shared_ptr<ShaderModule>>& modules);
            ~Shader();

            struct UBO
            {
                    SpvReflectDescriptorBinding descriptor_binding;
                    std::vector<u8> data;

                    // @TODO: frames in flight
                    vk::DescriptorSet descriptor_set;
                    vk::DescriptorSetLayout descriptor_set_layout;
                    Buffer buffer;
            };

            void add_attribute(const vk::Format format, const u32 size, const u32 offset);

            void set_uniform(const str& scope, const str& name, const void* data);

            void bind(const Pipeline& pipeline);
            void bind_texture(const Pipeline& pipeline, const str& name, const vk::DescriptorSet& descriptor_set);

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

            const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts() const
            {
                return descriptor_set_layouts;
            };

        private:
            std::vector<std::shared_ptr<ShaderModule>> modules;
            std::vector<vk::VertexInputBindingDescription> vertex_bindings = {};
            std::vector<vk::VertexInputAttributeDescription> vertex_attributes = {};
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

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
