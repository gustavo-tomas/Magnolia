#pragma once

// !TODO: use caching for render passes and pipelines!

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
            u64 offset;
    };

    // DescriptorBuilder
    // -----------------------------------------------------------------------------------------------------------------
    class DescriptorBuilder
    {
        public:
            static Descriptor build_layout(const SpvReflectShaderModule& shader_reflection, const u32 set);

            static void build(const Descriptor& descriptor, const std::vector<Buffer>& data_buffers);
            static void build(const Descriptor& descriptor, const std::vector<Image>& images);
    };
};  // namespace mag
