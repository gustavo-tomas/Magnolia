#include "renderer/descriptors.hpp"

#include <algorithm>

#include "core/logger.hpp"
#include "renderer/context.hpp"

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

        DescriptorLayoutInfo layoutinfo = {};
        layoutinfo.bindings.reserve(info->bindingCount);
        b8 is_sorted = true;
        i32 last_binding = -1;

        // copy from the direct info struct into our own one
        for (u32 i = 0; i < info->bindingCount; i++)
        {
            layoutinfo.bindings.push_back(info->pBindings[i]);

            // check that the bindings are in strict increasing order
            if (static_cast<i32>(info->pBindings[i].binding) > last_binding)
                last_binding = info->pBindings[i].binding;

            else
                is_sorted = false;
        }

        // sort the bindings if they aren't in order
        if (!is_sorted)
        {
            std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(),
                      [](vk::DescriptorSetLayoutBinding& a, vk::DescriptorSetLayoutBinding& b)
                      { return a.binding < b.binding; });
        }

        // try to grab from cache
        auto it = layout_cache.find(layoutinfo);
        if (it != layout_cache.end())
            return it->second;

        else
        {
            // create a new one (not found)
            vk::DescriptorSetLayout layout;
            VK_CHECK(context.get_device().createDescriptorSetLayout(info, nullptr, &layout));

            // add to cache
            layout_cache[layoutinfo] = layout;
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
    DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* layout_cache)
    {
        DescriptorBuilder descriptor_builder;
        descriptor_builder.cache = layout_cache;

        return descriptor_builder;
    }

    DescriptorBuilder& DescriptorBuilder::bind(const Shader::SpvReflection& shader_reflection)
    {
        // create the descriptor binding for the layout
        vk::DescriptorSetLayoutBinding new_binding(shader_reflection.binding, shader_reflection.descriptor_type, 1,
                                                   shader_reflection.shader_stage);

        bindings.push_back(new_binding);

        return *this;
    }

    void DescriptorBuilder::build(vk::DescriptorSetLayout& layout, Buffer* descriptor_buffer, const Buffer& data_buffer)
    {
        auto& context = get_context();
        auto& physical_device = context.get_physical_device();
        auto& device = context.get_device();

        // Build layout first
        const vk::DescriptorSetLayoutCreateInfo layout_info(vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
                                                            bindings);
        layout = cache->create_descriptor_layout(&layout_info);

        // @TODO: only need to be done once
        // 1. Get properties
        vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties;
        vk::PhysicalDeviceProperties2 device_properties;
        device_properties.pNext = &descriptor_buffer_properties;

        physical_device.getProperties2(&device_properties);

        // 2. Get size
        // Get set layout descriptor sizes.
        u64 size = device.getDescriptorSetLayoutSizeEXT(layout);
        u64 alignment = descriptor_buffer_properties.descriptorBufferOffsetAlignment;

        // Adjust set layout sizes to satisfy alignment requirements.
        size = (size + alignment - 1) & ~(alignment - 1);

        LOG_WARNING("SIZE: {0}", size);

        // Get descriptor bindings offsets as descriptors are placed inside set layout by those offsets.
        // u64 offset = device.getDescriptorSetLayoutBindingOffsetEXT(layout, bindings[0].binding);

        // 4. Put the descriptors into buffers

        // Global matrices uniform buffer
        const vk::DescriptorAddressInfoEXT address_info(data_buffer.get_device_address(), data_buffer.get_size(),
                                                        vk::Format::eUndefined);

        const vk::DescriptorGetInfoEXT buffer_descriptor_info(vk::DescriptorType::eUniformBuffer, {&address_info});

        device.getDescriptorEXT(buffer_descriptor_info, descriptor_buffer_properties.uniformBufferDescriptorSize,
                                descriptor_buffer->get_data());
    }
};  // namespace mag
