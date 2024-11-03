#pragma once

#include <unordered_map>
#include <vector>

#include "core/types.hpp"
#include "private/vulkan_fwd.hpp"

namespace mag
{
    // DescriptorAllocator
    // ---------------------------------------------------------------------------------------------------------------------
    class DescriptorAllocator
    {
        public:
            DescriptorAllocator();
            ~DescriptorAllocator();

            using PoolSizes = std::vector<std::pair<vk::DescriptorType, f32>>;

            b8 allocate(vk::DescriptorSet* set, const vk::DescriptorSetLayout layout);
            void reset_pools();

        private:
            vk::DescriptorPool grab_pool();

            struct IMPL;
            unique<IMPL> impl;
    };

    // DescriptorLayoutCache
    // ---------------------------------------------------------------------------------------------------------------------
    class DescriptorLayoutCache
    {
        public:
            DescriptorLayoutCache();
            ~DescriptorLayoutCache();

            vk::DescriptorSetLayout create_descriptor_layout(const vk::DescriptorSetLayoutCreateInfo* info);
            struct DescriptorLayoutInfo
            {
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

    // DescriptorBuilder
    // ---------------------------------------------------------------------------------------------------------------------
    class RendererImage;
    class VulkanBuffer;
    class DescriptorBuilder
    {
        public:
            DescriptorBuilder();
            ~DescriptorBuilder();

            static DescriptorBuilder begin(DescriptorLayoutCache* layout_cache, DescriptorAllocator* allocator);

            DescriptorBuilder& bind(const u32 binding, const vk::DescriptorType type,
                                    const vk::ShaderStageFlags stage_flags,
                                    const std::vector<vk::DescriptorBufferInfo>& infos);

            DescriptorBuilder& bind(const u32 binding, const vk::DescriptorType type,
                                    const vk::ShaderStageFlags stage_flags,
                                    const std::vector<vk::DescriptorImageInfo>& infos);

            b8 build(vk::DescriptorSet& set, vk::DescriptorSetLayout& layout);
            b8 build(vk::DescriptorSet& set);

            // Helpers
            static void create_descriptor_for_buffer(const u32 binding, vk::DescriptorSet& descriptor_set,
                                                     vk::DescriptorSetLayout& descriptor_set_layout,
                                                     const vk::DescriptorType type, const VulkanBuffer& buffer,
                                                     const u64 buffer_size, const u64 offset);

            static void create_descriptor_for_textures(const u32 binding,
                                                       const std::vector<ref<RendererImage>>& textures,
                                                       vk::DescriptorSet& descriptor_set,
                                                       vk::DescriptorSetLayout& descriptor_set_layout);

        private:
            std::vector<vk::WriteDescriptorSet> writes;
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            // These need to stay alive until the build is done
            std::vector<std::vector<vk::DescriptorBufferInfo>*> buffer_infos;
            std::vector<std::vector<vk::DescriptorImageInfo>*> image_infos;

            DescriptorLayoutCache* cache = nullptr;
            DescriptorAllocator* alloc = nullptr;
    };
};  // namespace mag
