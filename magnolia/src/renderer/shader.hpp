#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/math.hpp"
#include "core/types.hpp"
#include "renderer/buffers.hpp"
#include "spirv_reflect.h"

namespace mag
{
    using namespace mag::math;

    class Pipeline;
    struct Image;
    struct Material;

    struct ShaderModule
    {
            str file_path = "";

            vk::ShaderModule module = {};
            SpvReflectShaderModule spv_module = {};
    };

    struct ShaderConfiguration
    {
            str name = "";
            str file_path = "";

            std::vector<ShaderModule> shader_modules;

            str topology;
            str polygon_mode;
            str cull_mode;

            b8 color_blend_enabled;
            str color_blend_op;
            str alpha_blend_op;
            str src_color_blend_factor;
            str dst_color_blend_factor;
            str src_alpha_blend_factor;
            str dst_alpha_blend_factor;
    };

    class Shader
    {
        public:
            Shader(const ShaderConfiguration& shader_configuration);
            ~Shader();

            void bind();

            void set_uniform(const str& scope, const str& name, const void* data, const u64 data_offset = 0);
            void set_texture(const str& name, Image* texture);
            void set_material(const str& name, Material* material);

            const ShaderConfiguration& get_shader_configuration() const { return configuration; }

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
                    std::vector<VulkanBuffer> buffers;
            };

            void add_attribute(const vk::Format format, const u32 size, const u32 offset);
            void bind_descriptor(const u32 set, const vk::DescriptorSet& descriptor_set);

            ShaderConfiguration configuration;
            std::vector<vk::VertexInputBindingDescription> vertex_bindings = {};
            std::vector<vk::VertexInputAttributeDescription> vertex_attributes = {};
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;

            unique<Pipeline> pipeline;

            u32 location = 0;
            u32 stride = 0;
            std::map<str, UBO> uniforms_map;

            // Keep track of the descriptors for each texture
            std::map<Image*, vk::DescriptorSet> texture_descriptor_sets;
            std::map<Material*, vk::DescriptorSet> material_descriptor_sets;
    };

    class ShaderManager
    {
        public:
            ref<Shader> get(const str& file_path);

        private:
            std::map<str, ref<Shader>> shaders;
    };
};  // namespace mag
