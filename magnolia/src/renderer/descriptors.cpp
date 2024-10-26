#include "renderer/descriptors.hpp"

#include <vulkan/vulkan.hpp>

#include "core/assert.hpp"
#include "renderer/buffers.hpp"
#include "renderer/context.hpp"
#include "renderer/renderer_image.hpp"

namespace mag
{
    vk::DescriptorPool create_pool(const DescriptorAllocator::PoolSizes& pool_sizes, const u32 count,
                                   const vk::DescriptorPoolCreateFlags flags)
    {
        auto& device = get_context().get_device();

        std::vector<vk::DescriptorPoolSize> sizes;
        sizes.reserve(pool_sizes.size());
        for (const auto& [type, size] : pool_sizes)
        {
            vk::DescriptorPoolSize descriptor_pool_size(type, static_cast<u32>(size * count));
            sizes.push_back(descriptor_pool_size);
        }

        const vk::DescriptorPoolCreateInfo pool_info(flags, count, sizes);
        const vk::DescriptorPool descriptor_pool = device.createDescriptorPool(pool_info);

        return descriptor_pool;
    }

    // DescriptorAllocator
    // ---------------------------------------------------------------------------------------------------------------------
    DescriptorAllocator::DescriptorAllocator() = default;

    DescriptorAllocator::~DescriptorAllocator()
    {
        auto& context = get_context();

        for (const auto& p : free_pools) vkDestroyDescriptorPool(context.get_device(), p, nullptr);
        for (const auto& p : used_pools) vkDestroyDescriptorPool(context.get_device(), p, nullptr);
    }

    b8 DescriptorAllocator::allocate(vk::DescriptorSet* set, const vk::DescriptorSetLayout layout)
    {
        auto& context = get_context();

        // initialize the currentPool handle if it's null
        if (current_pool == VK_NULL_HANDLE)
        {
            current_pool = grab_pool();
            used_pools.push_back(current_pool);
        }

        vk::DescriptorSetAllocateInfo alloc_info(current_pool, 1, &layout);

        // try to allocate the descriptor set
        vk::Result alloc_result = context.get_device().allocateDescriptorSets(&alloc_info, set);
        b8 needReallocate = false;

        switch (alloc_result)
        {
            case vk::Result::eSuccess:
                return true;

            // reallocate pool
            case vk::Result::eErrorFragmentedPool:
            case vk::Result::eErrorOutOfPoolMemory:
                needReallocate = true;
                break;

            // unrecoverable error
            default:
                return false;
        }

        if (needReallocate)
        {
            // allocate a new pool and retry
            current_pool = grab_pool();
            used_pools.push_back(current_pool);

            alloc_result = context.get_device().allocateDescriptorSets(&alloc_info, set);

            if (alloc_result == vk::Result::eSuccess) return true;
        }

        // if it still fails then we have big issues
        return false;
    }

    void DescriptorAllocator::reset_pools()
    {
        auto& context = get_context();

        // reset all used pools and add them to the free pools
        for (const auto& p : used_pools)
        {
            VK_CHECK(VK_CAST(vkResetDescriptorPool(context.get_device(), p, 0)));
            free_pools.push_back(p);
        }

        // clear the used pools, since we've put them all in the free pools
        used_pools.clear();

        // reset the current pool handle back to null
        current_pool = VK_NULL_HANDLE;
    }

    vk::DescriptorPool DescriptorAllocator::grab_pool()
    {
        // there are reusable pools availible
        if (free_pools.size() > 0)
        {
            // grab pool from the back of the vector and remove it from there.
            vk::DescriptorPool pool = free_pools.back();
            free_pools.pop_back();
            return pool;
        }

        // no pools availible, so create a new one
        return create_pool(descriptor_sizes, 1000, {});
    }

    // DescriptorLayoutCache
    // ---------------------------------------------------------------------------------------------------------------------
    DescriptorLayoutCache::DescriptorLayoutCache() = default;

    DescriptorLayoutCache::~DescriptorLayoutCache()
    {
        auto& context = get_context();

        // delete every descriptor layout held
        for (const auto& layout_p : layout_cache)
        {
            const auto& layout = layout_p.second;
            context.get_device().destroyDescriptorSetLayout(layout);
        }
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
                      [](const vk::DescriptorSetLayoutBinding& a, const vk::DescriptorSetLayoutBinding& b)
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

        // @TODO: double check this algorithm
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
    // ---------------------------------------------------------------------------------------------------------------------
    DescriptorBuilder::DescriptorBuilder() = default;

    DescriptorBuilder::~DescriptorBuilder()
    {
        for (auto* info : buffer_infos)
        {
            delete info;
        }

        for (auto* info : image_infos)
        {
            delete info;
        }
    }

    DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator)
    {
        DescriptorBuilder builder;
        builder.cache = layoutCache;
        builder.alloc = allocator;

        return builder;
    }

    DescriptorBuilder& DescriptorBuilder::bind(const u32 binding, const vk::DescriptorType type,
                                               const vk::ShaderStageFlags stage_flags,
                                               const std::vector<vk::DescriptorBufferInfo>& infos)
    {
        std::vector<vk::DescriptorBufferInfo>* p_infos = new std::vector<vk::DescriptorBufferInfo>(infos);
        buffer_infos.push_back(p_infos);

        // create the descriptor binding for the layout
        vk::DescriptorSetLayoutBinding new_binding(binding, type, infos.size(), stage_flags);
        bindings.push_back(new_binding);

        // create the descriptor write
        vk::WriteDescriptorSet new_write({}, binding, {}, type, {}, *p_infos);
        writes.push_back(new_write);

        return *this;
    }

    DescriptorBuilder& DescriptorBuilder::bind(const u32 binding, const vk::DescriptorType type,
                                               const vk::ShaderStageFlags stage_flags,
                                               const std::vector<vk::DescriptorImageInfo>& infos)
    {
        std::vector<vk::DescriptorImageInfo>* p_infos = new std::vector<vk::DescriptorImageInfo>(infos);
        image_infos.push_back(p_infos);

        // create the descriptor binding for the layout
        vk::DescriptorSetLayoutBinding new_binding(binding, type, infos.size(), stage_flags);
        bindings.push_back(new_binding);

        // create the descriptor write
        vk::WriteDescriptorSet new_write({}, binding, {}, type, *p_infos);
        writes.push_back(new_write);

        return *this;
    }

    b8 DescriptorBuilder::build(vk::DescriptorSet& set, vk::DescriptorSetLayout& layout)
    {
        // build layout first
        vk::DescriptorSetLayoutCreateInfo layout_info({}, bindings);
        layout = cache->create_descriptor_layout(&layout_info);

        // allocate descriptor
        b8 success = alloc->allocate(&set, layout);
        if (!success) return false;

        // write descriptor
        return this->build(set);
    }

    b8 DescriptorBuilder::build(vk::DescriptorSet& set)
    {
        auto& context = get_context();

        // write descriptor
        for (vk::WriteDescriptorSet& w : writes) w.setDstSet(set);

        context.get_device().updateDescriptorSets(writes, {});

        return true;
    }

    void DescriptorBuilder::create_descriptor_for_buffer(const u32 binding, vk::DescriptorSet& descriptor_set,
                                                         vk::DescriptorSetLayout& descriptor_set_layout,
                                                         const vk::DescriptorType type, const VulkanBuffer& buffer,
                                                         const u64 buffer_size, const u64 offset)
    {
        // Create descriptors for this buffer
        auto& descriptor_layout_cache = get_context().get_descriptor_layout_cache();
        auto& descriptor_allocator = get_context().get_descriptor_allocator();

        const vk::DescriptorBufferInfo descriptor_buffer_info(buffer.get_buffer(), offset, buffer_size);

        const b8 result =
            DescriptorBuilder::begin(&descriptor_layout_cache, &descriptor_allocator)
                .bind(binding, type, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                      {descriptor_buffer_info})
                .build(descriptor_set, descriptor_set_layout);

        ASSERT(result, "Failed to build descriptor of type '{0}'", vk::to_string(type));
    }

    void DescriptorBuilder::create_descriptor_for_textures(const u32 binding,
                                                           const std::vector<ref<RendererImage>>& textures,
                                                           vk::DescriptorSet& descriptor_set,
                                                           vk::DescriptorSetLayout& descriptor_set_layout)
    {
        // Create descriptors for this texture
        auto& descriptor_layout_cache = get_context().get_descriptor_layout_cache();
        auto& descriptor_allocator = get_context().get_descriptor_allocator();

        std::vector<vk::DescriptorImageInfo> descriptor_image_infos;

        for (auto& texture : textures)
        {
            const vk::DescriptorImageInfo descriptor_image_info(
                *static_cast<const vk::Sampler*>(texture->get_sampler().get_handle()), texture->get_image_view(),
                vk::ImageLayout::eReadOnlyOptimal);

            descriptor_image_infos.push_back(descriptor_image_info);
        }

        const b8 result =
            DescriptorBuilder::begin(&descriptor_layout_cache, &descriptor_allocator)
                .bind(binding, vk::DescriptorType::eCombinedImageSampler,
                      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, descriptor_image_infos)
                .build(descriptor_set, descriptor_set_layout);

        ASSERT(result, "Failed to build descriptor for textures");
    }
};  // namespace mag
