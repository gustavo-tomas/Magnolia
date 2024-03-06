#pragma once

// !TODO: use caching for render passes and pipelines!

#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "renderer/buffers.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    class Context;

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
            Descriptor build_layout(const SpvReflectShaderModule& shader_reflection, const u32 set);

            void build(const Descriptor& descriptor, const Buffer& global_buffer, const Buffer& model_buffer);

        private:
            vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties;
            vk::PhysicalDeviceProperties2 device_properties;
    };
};  // namespace mag
