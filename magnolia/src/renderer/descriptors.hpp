#pragma once

// !TODO: use caching for render passes and pipelines!

#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    class Context;

    // DescriptorAllocator
    // -----------------------------------------------------------------------------------------------------------------
    class DescriptorAllocator
    {
        public:
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

            void initialize();
            void shutdown();

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

    // DescriptorBuilder
    // -----------------------------------------------------------------------------------------------------------------
    class DescriptorBuilder
    {
        public:
            static DescriptorBuilder begin(DescriptorLayoutCache* layout_cache, DescriptorAllocator* allocator);

            DescriptorBuilder& bind(const Shader::SpvReflection& shader_reflection,
                                    const vk::DescriptorBufferInfo* buffer_info);

            DescriptorBuilder& bind(const Shader::SpvReflection& shader_reflection,
                                    const vk::DescriptorImageInfo* image_info);

            b8 build(vk::DescriptorSet& set, vk::DescriptorSetLayout& layout);
            b8 build(vk::DescriptorSet& set);

        private:
            std::vector<vk::WriteDescriptorSet> writes;
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            DescriptorLayoutCache* cache = {};
            DescriptorAllocator* alloc = {};
    };
};  // namespace mag
