#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "nlohmann/json.hpp"
#include "renderer/buffers.hpp"
#include "spirv_reflect.h"

namespace mag
{
    using namespace mag::math;
    using json = nlohmann::ordered_json;

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
            Shader(const std::vector<std::shared_ptr<ShaderModule>>& modules, const json pipeline_data);
            ~Shader();

            void set_uniform(const str& scope, const str& name, const void* data, const u64 data_offset = 0);
            void bind_texture(const str& name, const vk::DescriptorSet& descriptor_set);

            void bind();

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
            struct UBO
            {
                    SpvReflectDescriptorBinding descriptor_binding;

                    // One per frame in flight
                    std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
                    std::vector<vk::DescriptorSet> descriptor_sets;
                    std::vector<Buffer> buffers;
            };

            void add_attribute(const vk::Format format, const u32 size, const u32 offset);

            std::vector<std::shared_ptr<ShaderModule>> modules;
            std::vector<vk::VertexInputBindingDescription> vertex_bindings = {};
            std::vector<vk::VertexInputAttributeDescription> vertex_attributes = {};
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

            std::unique_ptr<Pipeline> pipeline;

            u32 location = 0;
            u32 stride = 0;
            std::map<str, UBO> uniforms_map;
    };

    class ShaderManager
    {
        public:
            ShaderManager() = default;
            ~ShaderManager();

            std::shared_ptr<Shader> load(const str& file_path);

        private:
            std::shared_ptr<ShaderModule> load_module(const str& file);
            std::map<str, std::shared_ptr<ShaderModule>> shader_modules;
            std::map<str, std::shared_ptr<Shader>> shaders;
    };
};  // namespace mag
