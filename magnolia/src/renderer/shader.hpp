#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/descriptors.hpp"
#include "spirv_reflect.h"

namespace mag
{
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

            void add_attribute(const vk::Format format, const u32 size, const u32 offset);
            void add_uniform(const str& name);

            // @TODO: this is temporary. We want to avoid needing to pass modules around
            const std::vector<std::shared_ptr<ShaderModule>>& get_modules() const { return modules; };

            const vk::VertexInputBindingDescription& get_vertex_binding() const { return binding; };

            std::vector<vk::DescriptorSetLayout> get_layouts()
            {
                std::vector<vk::DescriptorSetLayout> layouts;
                for (auto& [set, descriptor] : descriptors)
                {
                    layouts.push_back(descriptor.layout);
                }

                return layouts;
            };

            std::map<u32, Descriptor>& get_descriptors() { return descriptors; };

            const std::vector<vk::VertexInputAttributeDescription>& get_vertex_attributes() const
            {
                return attributes;
            };

        private:
            void add_binding(const u32 set, const SpvReflectDescriptorBinding& descriptor_binding,
                             const vk::ShaderStageFlags stage);
            void build_layout(const u32 set);

            std::vector<std::shared_ptr<ShaderModule>> modules = {};
            vk::VertexInputBindingDescription binding = {};
            std::vector<vk::VertexInputAttributeDescription> attributes = {};
            std::map<u32, Descriptor> descriptors = {};
            std::map<u32, std::vector<vk::DescriptorSetLayoutBinding>> layout_bindings = {};

            u32 location = 0;
            u32 stride = 0;
            vk::ShaderStageFlags stages = {};
    };

    class ShaderLoader
    {
        public:
            ShaderLoader() = default;
            ~ShaderLoader();

            std::shared_ptr<Shader> load(const str& name, const str& vertex_file, const str& fragment_file);

        private:
            std::shared_ptr<ShaderModule> load_module(const str& file);
            std::map<str, std::shared_ptr<ShaderModule>> shader_modules;
            std::map<str, std::shared_ptr<Shader>> shaders;
    };
};  // namespace mag
