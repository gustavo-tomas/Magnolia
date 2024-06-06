#pragma once

// @TODO: imagine how good this will look once i refactor it

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/buffers.hpp"
#include "spirv_reflect.h"

namespace mag
{
    class Context;
    class Image;
    enum class TextureType;

    // DescriptorLayoutCache
    // -----------------------------------------------------------------------------------------------------------------
    class DescriptorLayoutCache
    {
        public:
            void initialize();
            void shutdown();

            vk::DescriptorSetLayout create_descriptor_layout(const vk::DescriptorSetLayoutCreateInfo* info);
            struct DescriptorLayoutInfo
            {
                    // good idea to turn this into a inlined array
                    std::vector<vk::DescriptorSetLayoutBinding> bindings;

                    b8 operator==(const DescriptorLayoutInfo& other) const;

                    size_t hash() const;
            };

        private:
            struct DescriptorLayoutHash
            {
                    std::size_t operator()(const DescriptorLayoutInfo& k) const;
            };

            std::unordered_map<DescriptorLayoutInfo, vk::DescriptorSetLayout, DescriptorLayoutHash> layout_cache;
    };

    struct Descriptor
    {
            vk::DescriptorSetLayout layout = {};
            Buffer buffer;
            u64 size;
            std::vector<u64> offsets;
    };

    // DescriptorBuilder
    // -----------------------------------------------------------------------------------------------------------------
    class DescriptorBuilder
    {
        public:
            static Descriptor build_layout(const SpvReflectShaderModule& shader_reflection, const u32 set);

            static void build(const Descriptor& descriptor, const std::vector<Buffer>& data_buffers);
            static void build(const Descriptor& descriptor, const std::vector<std::shared_ptr<Image>>& images);
    };

    // DescriptorCache
    // -----------------------------------------------------------------------------------------------------------------
    struct Model;
    class Shader;
    class DescriptorCache
    {
        public:
            DescriptorCache() = default;
            ~DescriptorCache();

            void build_descriptors_from_shader(const Shader& shader);
            void bind();
            void add_image_descriptors_for_model(const Model& model);

            void set_offset_global(const vk::PipelineLayout& pipeline_layout);
            void set_offset_instance(const vk::PipelineLayout& pipeline_layout, const u32 instance);
            void set_offset_material(const vk::PipelineLayout& pipeline_layout, const u32 index,
                                     const TextureType texture_type);
            void set_offset_shader(const vk::PipelineLayout& pipeline_layout);

            // @TODO: idk
            std::vector<std::vector<Buffer>>& get_data_buffers() { return data_buffers; };
            std::vector<Buffer>& get_shader_data_buffers() { return shader_data_buffers; };

            const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts() const
            {
                return descriptor_set_layouts;
            };

            const std::vector<std::shared_ptr<Image>>& get_albedo_textures() const { return albedo_textures; };
            const std::vector<std::shared_ptr<Image>>& get_normal_textures() const { return normal_textures; };

        private:
            void set_descriptor_buffer_offset(const vk::PipelineLayout& pipeline_layout, const u32 first_set,
                                              const u32 buffer_indices, const u64 buffer_offsets);

            std::vector<std::vector<Buffer>> data_buffers;
            std::vector<Buffer> shader_data_buffers;
            std::vector<std::shared_ptr<Image>> albedo_textures, normal_textures;
            std::vector<Descriptor> uniform_descriptors, shader_uniform_descriptors, albedo_image_descriptors,
                normal_image_descriptors;
            std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
    };
};  // namespace mag
