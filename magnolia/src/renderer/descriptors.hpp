#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"

namespace mag
{
    // DescriptorAllocator
    // ---------------------------------------------------------------------------------------------------------------------
    class DescriptorAllocator
    {
        public:
            DescriptorAllocator() = default;
            ~DescriptorAllocator();

            std::vector<std::pair<vk::DescriptorType, f32>> sizes = {{vk::DescriptorType::eSampler, 0.5f},
                                                                     {vk::DescriptorType::eCombinedImageSampler, 4.0f},
                                                                     {vk::DescriptorType::eSampledImage, 4.0f},
                                                                     {vk::DescriptorType::eStorageImage, 1.0f},
                                                                     {vk::DescriptorType::eUniformTexelBuffer, 1.0f},
                                                                     {vk::DescriptorType::eStorageTexelBuffer, 1.0f},
                                                                     {vk::DescriptorType::eUniformBuffer, 2.0f},
                                                                     {vk::DescriptorType::eStorageBuffer, 2.0f},
                                                                     {vk::DescriptorType::eUniformBufferDynamic, 1.0f},
                                                                     {vk::DescriptorType::eStorageBufferDynamic, 1.0f},
                                                                     {vk::DescriptorType::eInputAttachment, 0.5f}};

            using PoolSizes = std::vector<std::pair<vk::DescriptorType, f32>>;

            b8 allocate(vk::DescriptorSet* set, const vk::DescriptorSetLayout layout);
            void reset_pools();

        private:
            vk::DescriptorPool grab_pool();

            vk::DescriptorPool current_pool = {};
            PoolSizes descriptor_sizes;
            std::vector<vk::DescriptorPool> used_pools;
            std::vector<vk::DescriptorPool> free_pools;
    };

    // DescriptorLayoutCache
    // ---------------------------------------------------------------------------------------------------------------------
    class DescriptorLayoutCache
    {
        public:
            DescriptorLayoutCache() = default;
            ~DescriptorLayoutCache();

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

    // DescriptorBuilder
    // ---------------------------------------------------------------------------------------------------------------------
    class Image;
    class Buffer;
    class DescriptorBuilder
    {
        public:
            static DescriptorBuilder begin(DescriptorLayoutCache* layout_cache, DescriptorAllocator* allocator);

            DescriptorBuilder& bind(const u32 binding, const vk::DescriptorType type,
                                    const vk::ShaderStageFlags stage_flags,
                                    const vk::DescriptorBufferInfo* buffer_info);

            DescriptorBuilder& bind(const u32 binding, const vk::DescriptorType type,
                                    const vk::ShaderStageFlags stage_flags, const vk::DescriptorImageInfo* buffer_info);

            b8 build(vk::DescriptorSet& set, vk::DescriptorSetLayout& layout);
            b8 build(vk::DescriptorSet& set);

            // Helpers
            static void create_descriptor_for_buffer(vk::DescriptorSet& descriptor_set,
                                                     vk::DescriptorSetLayout& descriptor_set_layout,
                                                     const vk::DescriptorType type, const Buffer& buffer,
                                                     const u64 buffer_size, const u64 offset);

            static void create_descriptor_for_texture(const u32 binding, const std::shared_ptr<Image>& texture,
                                                      vk::DescriptorSet& descriptor_set,
                                                      vk::DescriptorSetLayout& descriptor_set_layout);

        private:
            std::vector<vk::WriteDescriptorSet> writes;
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            DescriptorLayoutCache* cache = nullptr;
            DescriptorAllocator* alloc = nullptr;
    };
};  // namespace mag
