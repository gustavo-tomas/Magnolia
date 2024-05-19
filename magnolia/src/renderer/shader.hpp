#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/buffers.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/model.hpp"
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
            ~Shader();

            void ugly_init();
            void add_attribute(const vk::Format format, const u32 size, const u32 offset);

            void set_uniform_global(const str& name, const void* data);
            void set_uniform_instance(const str& name, const void* data, const u32 instance);

            void set_offset_global(const vk::PipelineLayout& pipeline_layout);
            void set_offset_instance(const vk::PipelineLayout& pipeline_layout, const u32 instance);
            void set_offset_material(const vk::PipelineLayout& pipeline_layout, const u32 index);

            void bind();

            void add_model(const Model& model);

            // @TODO: this is temporary. We want to avoid needing to pass modules around
            const std::vector<std::shared_ptr<ShaderModule>>& get_modules() const { return modules; };

            const vk::VertexInputBindingDescription& get_vertex_binding() const { return vertex_binding; };

            const std::vector<vk::VertexInputAttributeDescription>& get_vertex_attributes() const
            {
                return vertex_attributes;
            };

            const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts() const
            {
                return descriptor_set_layouts;
            };

        private:
            void set_uniform(const str& scope, const str& name, const void* data, const u32 buffer_index);
            void set_descriptor_buffer_offset(const vk::PipelineLayout& pipeline_layout, const u32 first_set,
                                              const u32 buffer_indices, const u64 buffer_offsets);

            struct UBO
            {
                    SpvReflectDescriptorBinding descriptor_binding;
                    std::vector<u8> data;
            };

            std::vector<std::shared_ptr<ShaderModule>> modules;
            vk::VertexInputBindingDescription vertex_binding = {};
            std::vector<vk::VertexInputAttributeDescription> vertex_attributes = {};

            u32 location = 0;
            u32 stride = 0;
            std::map<str, UBO> uniforms_map;

            // @TODO: refactor
            std::vector<std::vector<Buffer>> data_buffers;
            std::vector<Image> textures;
            std::vector<Descriptor> uniform_descriptors, image_descriptors;
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
            // @TODO: refactor
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
