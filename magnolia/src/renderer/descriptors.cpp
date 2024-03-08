#include "renderer/descriptors.hpp"

#include <algorithm>

#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"

namespace mag
{
    // DescriptorLayoutCache
    // -----------------------------------------------------------------------------------------------------------------
    void DescriptorLayoutCache::initialize() {}

    void DescriptorLayoutCache::shutdown()
    {
        auto& context = get_context();

        // delete every descriptor layout held
        for (const auto& [info, layout] : layout_cache) context.get_device().destroyDescriptorSetLayout(layout);
    }

    vk::DescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(
        const vk::DescriptorSetLayoutCreateInfo* info)
    {
        auto& context = get_context();

        DescriptorLayoutInfo layout_info = {};
        layout_info.bindings.reserve(info->bindingCount);
        b8 is_sorted = true;
        i32 last_binding = -1;

        // copy from the direct info struct into our own one
        for (u32 i = 0; i < info->bindingCount; i++)
        {
            layout_info.bindings.push_back(info->pBindings[i]);

            // check that the bindings are in strict increasing order
            if (static_cast<i32>(info->pBindings[i].binding) > last_binding)
                last_binding = info->pBindings[i].binding;

            else
                is_sorted = false;
        }

        // sort the bindings if they aren't in order
        if (!is_sorted)
        {
            std::sort(layout_info.bindings.begin(), layout_info.bindings.end(),
                      [](vk::DescriptorSetLayoutBinding& a, vk::DescriptorSetLayoutBinding& b)
                      { return a.binding < b.binding; });
        }

        // try to grab from cache
        auto it = layout_cache.find(layout_info);
        if (it != layout_cache.end())
            return it->second;

        else
        {
            // create a new one (not found)
            vk::DescriptorSetLayout layout;
            VK_CHECK(context.get_device().createDescriptorSetLayout(info, nullptr, &layout));

            // add to cache
            layout_cache[layout_info] = layout;
            return layout;
        }
    }

    b8 DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
    {
        if (other.bindings.size() != bindings.size()) return false;

        // compare each of the bindings is the same. Bindings are sorted so they will match
        else
        {
            for (u64 i = 0; i < bindings.size(); i++)
            {
                if (other.bindings[i].binding != bindings[i].binding) return false;
                if (other.bindings[i].descriptorType != bindings[i].descriptorType) return false;
                if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) return false;
                if (other.bindings[i].stageFlags != bindings[i].stageFlags) return false;
            }

            return true;
        }
    }

    size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
    {
        using std::hash;
        using std::size_t;

        // !TODO: double check this algorithm
        size_t result = hash<size_t>()(bindings.size());
        for (const vk::DescriptorSetLayoutBinding& b : bindings)
        {
            // pack the binding data into a single int64. Not fully correct but it's ok
            size_t binding_hash = b.binding | static_cast<u32>(b.descriptorType) << 8 | b.descriptorCount << 16 |
                                  static_cast<u32>(b.stageFlags) << 24;

            // shuffle the packed binding data and xor it with the main hash
            result ^= hash<size_t>()(binding_hash);
        }

        return result;
    }

    size_t DescriptorLayoutCache::DescriptorLayoutHash::operator()(const DescriptorLayoutInfo& k) const
    {
        return k.hash();
    }

    // DescriptorBuilder
    // -----------------------------------------------------------------------------------------------------------------
    Descriptor DescriptorBuilder::build_layout(const SpvReflectShaderModule& shader_reflection, const u32 set)
    {
        ASSERT(shader_reflection.descriptor_set_count > set, "Invalid descriptor set");

        auto& context = get_context();
        auto& physical_device = context.get_physical_device();
        auto& device = context.get_device();
        auto& cache = context.get_descriptor_cache();

        Descriptor descriptor;

        const vk::ShaderStageFlagBits stage = static_cast<vk::ShaderStageFlagBits>(shader_reflection.shader_stage);

        // Create the descriptor binding for the layout
        const SpvReflectDescriptorSet& descriptor_set = shader_reflection.descriptor_sets[set];

        // Create all bindings for the specified set
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        for (u32 b = 0; b < descriptor_set.binding_count; b++)
        {
            const u32 binding = descriptor_set.bindings[b]->binding;
            const vk::DescriptorType type =
                static_cast<vk::DescriptorType>(descriptor_set.bindings[b]->descriptor_type);

            vk::DescriptorSetLayoutBinding new_binding(binding, type, 1, stage);
            bindings.push_back(new_binding);
        }

        // Build layout
        const vk::DescriptorSetLayoutCreateInfo layout_info(vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
                                                            bindings);
        descriptor.layout = cache.create_descriptor_layout(&layout_info);

        // @TODO: only needs to be done once
        // 1. Get properties
        device_properties.pNext = &descriptor_buffer_properties;
        physical_device.getProperties2(&device_properties);

        // 2. Get size
        // Get set layout descriptor sizes and adjust them to satisfy alignment requirements.
        const u64 alignment = descriptor_buffer_properties.descriptorBufferOffsetAlignment;
        descriptor.size = device.getDescriptorSetLayoutSizeEXT(descriptor.layout);
        descriptor.size = (descriptor.size + alignment - 1) & ~(alignment - 1);

        // Get descriptor bindings offsets as descriptors are placed inside set layout by those offsets.
        descriptor.offset = device.getDescriptorSetLayoutBindingOffsetEXT(descriptor.layout, 0);

        return descriptor;
    }

    void DescriptorBuilder::build(const Descriptor& descriptor, const std::vector<Buffer>& data_buffers)
    {
        auto& device = get_context().get_device();

        // 4. Put the descriptors into buffers
        char* descriptor_buffer_data = static_cast<char*>(descriptor.buffer.get_data());

        for (u64 i = 0; i < data_buffers.size(); i++)
        {
            const vk::DescriptorAddressInfoEXT address_info(data_buffers[i].get_device_address(),
                                                            data_buffers[i].get_size(), vk::Format::eUndefined);

            const vk::DescriptorGetInfoEXT descriptor_info(vk::DescriptorType::eUniformBuffer, {&address_info});

            const u64 offset = i ? i * descriptor.size + descriptor.offset : 0;

            device.getDescriptorEXT(descriptor_info, descriptor_buffer_properties.uniformBufferDescriptorSize,
                                    descriptor_buffer_data + offset);
        }
    }

    void DescriptorBuilder::build(const Descriptor& descriptor, const std::vector<Image>& images)
    {
        auto& device = get_context().get_device();

        // 4. Put the descriptors into buffers
        char* descriptor_buffer_data = static_cast<char*>(descriptor.buffer.get_data());

        for (u64 i = 0; i < images.size(); i++)
        {
            const vk::DescriptorImageInfo image_info(images[i].get_sampler().get_handle(), images[i].get_image_view());

            const vk::DescriptorGetInfoEXT descriptor_info(vk::DescriptorType::eCombinedImageSampler, {&image_info});

            const u64 offset = i * descriptor.size + descriptor.offset;

            device.getDescriptorEXT(descriptor_info, descriptor_buffer_properties.combinedImageSamplerDescriptorSize,
                                    descriptor_buffer_data + offset);
        }
    }
};  // namespace mag
