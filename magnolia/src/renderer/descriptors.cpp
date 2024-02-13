#include "renderer/descriptors.hpp"

#include <algorithm>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    vk::DescriptorPool createPool(const vk::Device device, const DescriptorAllocator::PoolSizes& pool_sizes,
                                  const u32 count, const vk::DescriptorPoolCreateFlags flags)
    {
        std::vector<vk::DescriptorPoolSize> sizes;
        sizes.reserve(pool_sizes.size());
        for (const auto& [type, size] : pool_sizes) sizes.push_back({type, static_cast<u32>(size * count)});

        const vk::DescriptorPoolCreateInfo pool_info(flags, count, sizes);
        const vk::DescriptorPool descriptor_pool = device.createDescriptorPool(pool_info);

        return descriptor_pool;
    }

    // DescriptorAllocator
    // -----------------------------------------------------------------------------------------------------------------
    void DescriptorAllocator::initialize() {}

    void DescriptorAllocator::shutdown()
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
        b8 need_reallocate = false;

        switch (alloc_result)
        {
            case vk::Result::eSuccess:
                return true;

            // reallocate pool
            case vk::Result::eErrorFragmentedPool:
            case vk::Result::eErrorOutOfPoolMemory:
                need_reallocate = true;
                break;

            // unrecoverable error
            default:
                return false;
        }

        if (need_reallocate)
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
        auto& context = get_context();

        // there are reusable pools availible
        if (free_pools.size() > 0)
        {
            // grab pool from the back of the vector and remove it from there.
            vk::DescriptorPool pool = free_pools.back();
            free_pools.pop_back();
            return pool;
        }

        // no pools availible, so create a new one
        return createPool(context.get_device(), descriptor_sizes, 1000, {});
    }

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
    DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache* layout_cache, DescriptorAllocator* allocator)
    {
        DescriptorBuilder builder;
        builder.cache = layout_cache;
        builder.alloc = allocator;

        return builder;
    }

    DescriptorBuilder& DescriptorBuilder::bind(const Shader::SpvReflection& shader_reflection,
                                               const vk::DescriptorBufferInfo* buffer_info)
    {
        // create the descriptor binding for the layout
        vk::DescriptorSetLayoutBinding new_binding(shader_reflection.binding, shader_reflection.descriptor_type, 1,
                                                   shader_reflection.shader_stage);
        bindings.push_back(new_binding);

        // create the descriptor write
        vk::WriteDescriptorSet new_write({}, shader_reflection.binding, {}, 1, shader_reflection.descriptor_type, {},
                                         buffer_info);
        writes.push_back(new_write);

        return *this;
    }

    DescriptorBuilder& DescriptorBuilder::bind(const Shader::SpvReflection& shader_reflection,
                                               const vk::DescriptorImageInfo* image_info)
    {
        // create the descriptor binding for the layout
        vk::DescriptorSetLayoutBinding new_binding(shader_reflection.binding, shader_reflection.descriptor_type, 1,
                                                   shader_reflection.shader_stage);
        bindings.push_back(new_binding);

        // create the descriptor write
        vk::WriteDescriptorSet new_write({}, shader_reflection.binding, {}, 1, shader_reflection.descriptor_type,
                                         image_info);
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
};  // namespace mag
