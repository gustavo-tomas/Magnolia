#include "../backend.hpp"
// This one comes first

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "VkBootstrap.h"
#include "VkBootstrapDispatch.h"
#include "conversions.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/debug.hpp"
#include "magnolia/core/memory.hpp"
#include "magnolia/math/functions.hpp"
#include "magnolia/platform/window.hpp"

// Use to trace VMA allocations
#if MAG_CONFIG_DEBUG_TRACE
    #define VMA_DEBUG_LOG_FORMAT(format, ...) \
        do                                    \
        {                                     \
            printf((format), __VA_ARGS__);    \
            printf("\n");                     \
        } while (false)

    #define VMA_DEBUG_LOG(str) VMA_DEBUG_LOG_FORMAT("%s", (str))
#endif

#define VMA_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

namespace mag
{
    constexpr void vk_check(const VkResult result, const str& message)
    {
        MAG_ASSERT((result) == VK_SUCCESS, "Vk check failed: '{}'", message);
    }

    namespace gfx
    {
        class VulkanSemaphore : public ISemaphore
        {
            public:
                VulkanSemaphore(const ISemaphoreDesc& desc, const vkb::DispatchTable* disp) : disp(disp)
                {
                    (void)desc;

                    VkSemaphoreCreateInfo semaphore_info = {};
                    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                    vk_check(disp->createSemaphore(&semaphore_info, nullptr, &semaphore), "Failed to create semaphore");
                }

                ~VulkanSemaphore() override { disp->destroySemaphore(semaphore, nullptr); }

                const VkSemaphore& get_semaphore() const { return semaphore; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkSemaphore semaphore = {};
        };

        class VulkanFence : public IFence
        {
            public:
                VulkanFence(const IFenceDesc& desc, const vkb::DispatchTable* disp) : disp(disp)
                {
                    VkFenceCreateInfo fence_info = {};
                    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

                    if (desc.signaled)
                    {
                        fence_info.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
                    }

                    vk_check(disp->createFence(&fence_info, nullptr, &fence), "Failed to create fence");
                }

                ~VulkanFence() override { disp->destroyFence(fence, nullptr); }

                void wait(const u64 timeout) const override { disp->waitForFences(1, &fence, VK_TRUE, timeout); }

                void reset() const override { disp->resetFences(1, &fence); }

                const VkFence& get_fence() const { return fence; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkFence fence = {};
        };

        class VulkanBuffer : public IBuffer
        {
            public:
                VulkanBuffer(const IBufferDesc& desc, VmaAllocator allocator)
                    : allocator(allocator), size(desc.size_bytes), usage(desc.buffer_usage)
                {
                    VkBufferCreateInfo buffer_create_info = {};
                    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    buffer_create_info.size = desc.size_bytes;
                    buffer_create_info.usage = mag_to_vk(desc.buffer_usage);

                    VmaAllocationCreateInfo allocation_create_info = {};
                    allocation_create_info.usage = mag_to_vk(desc.memory_usage);
                    allocation_create_info.flags =
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

                    vk_check(vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &buffer,
                                             &allocation, nullptr),
                             "Failed to create buffer");

                    // Use persistent mapping
                    vk_check(vmaMapMemory(allocator, allocation, &mapped_region), "Failed to map buffer memory");
                }

                ~VulkanBuffer() override
                {
                    vmaUnmapMemory(allocator, allocation);
                    vmaDestroyBuffer(allocator, buffer, allocation);
                }

                void* map() override
                {
                    vk_check(vmaMapMemory(allocator, allocation, &mapped_region), "Failed to map buffer memory");
                    return mapped_region;
                }

                void unmap() const override { vmaUnmapMemory(allocator, allocation); }

                void set_data(const void* const data, const u64 data_size, const u64 offset) const override
                {
                    MAG_ASSERT(offset + data_size <= size, "Size limit exceeded");
                    mem::copy(static_cast<c8*>(mapped_region) + offset, size, data, data_size, data_size);
                }

                u64 get_size() const override { return size; }

                BufferUsage get_usage() const override { return usage; }

                void* get_mapped_region() const override { return mapped_region; }

                const VkBuffer& get_buffer() const { return buffer; }

            private:
                VmaAllocator allocator = nullptr;
                u64 size = 0;
                BufferUsage usage;
                VkBuffer buffer = {};
                VmaAllocation allocation = nullptr;
                void* mapped_region = nullptr;
        };

        class VulkanSampler : public ISampler
        {
            public:
                VulkanSampler(const ISamplerDesc& desc, const vkb::DispatchTable* disp) : disp(disp)
                {
                    VkSamplerCreateInfo sampler_info = {};
                    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                    sampler_info.minFilter = mag_to_vk(desc.min_filter);
                    sampler_info.magFilter = mag_to_vk(desc.mag_filter);
                    sampler_info.mipmapMode = mag_to_vk(desc.mipmap_mode);
                    sampler_info.addressModeU = mag_to_vk(desc.address_mode_u);
                    sampler_info.addressModeV = mag_to_vk(desc.address_mode_v);
                    sampler_info.addressModeW = mag_to_vk(desc.address_mode_w);
                    sampler_info.minLod = desc.min_lod;
                    sampler_info.maxLod = desc.max_lod;
                    sampler_info.anisotropyEnable = static_cast<VkBool32>(desc.anisotropy_enable);
                    sampler_info.maxAnisotropy = desc.max_anisotropy;

                    disp->createSampler(&sampler_info, nullptr, &sampler);
                }

                ~VulkanSampler() override { disp->destroySampler(sampler, nullptr); }

                const VkSampler& get_sampler() const { return sampler; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkSampler sampler = {};
        };

        class VulkanTexture : public ITexture
        {
            public:
                VulkanTexture(const ITextureDesc& desc, const vkb::DispatchTable* disp, IDevice* device,
                              VmaAllocator allocator)
                    : disp(disp),
                      allocator(allocator),
                      device(device),
                      extent(desc.extent),
                      format(desc.format),
                      type(desc.type),
                      view_type(desc.view_type),
                      aspect(desc.aspect),
                      usage(desc.usage),
                      mip_levels(desc.mip_levels),
                      array_layers(desc.array_layers),
                      sample_count(desc.sample_count)
                {
                    // Create image and image view
                    const VkImageCreateInfo image_create_info = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = 0,
                        .imageType = mag_to_vk(desc.type),
                        .format = mag_to_vk(desc.format),
                        .extent = mag_to_vk(desc.extent),
                        .mipLevels = desc.mip_levels,
                        .arrayLayers = desc.array_layers,
                        .samples = mag_to_vk(desc.sample_count),
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage = mag_to_vk(desc.usage),
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 0,
                        .pQueueFamilyIndices = nullptr,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    };

                    VmaAllocationCreateInfo vma_alloc_info = {};
                    vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                    vma_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                    vk_check(
                        vmaCreateImage(allocator, &image_create_info, &vma_alloc_info, &image, &allocation, nullptr),
                        "Failed to create image");

                    VkImageViewCreateInfo view_create_info = {};
                    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    view_create_info.image = image;
                    view_create_info.format = mag_to_vk(desc.format);
                    view_create_info.viewType = mag_to_vk(desc.view_type);
                    view_create_info.subresourceRange.aspectMask = mag_to_vk(desc.aspect);
                    view_create_info.subresourceRange.baseArrayLayer = 0;
                    view_create_info.subresourceRange.baseMipLevel = 0;
                    view_create_info.subresourceRange.layerCount = 1;
                    view_create_info.subresourceRange.levelCount = 1;

                    vk_check(disp->createImageView(&view_create_info, nullptr, &image_view),
                             "Failed to create image view");
                }

                // Special case for swapchain images
                VulkanTexture(const vkb::DispatchTable* disp, VmaAllocator allocator, const math::uvec3& extent,
                              VkImage image, VkImageView image_view)
                    : disp(disp),
                      allocator(allocator),
                      image(image),
                      image_view(image_view),
                      extent(extent),
                      usage(TextureUsage::TransferDst)
                {
                }

                ~VulkanTexture() override
                {
                    disp->destroyImageView(image_view, nullptr);
                    if (allocation != nullptr)
                    {
                        vmaDestroyImage(allocator, image, allocation);
                    }
                }

                void set_data(const void* const data, const u64 size) override
                {
                    IBufferDesc staging_buffer_desc = {};
                    staging_buffer_desc.buffer_usage = BufferUsage::TransferSrc;
                    staging_buffer_desc.memory_usage = MemoryUsage::Auto;
                    staging_buffer_desc.size_bytes = size;

                    unique<IBuffer> staging_buffer = device->create_buffer(staging_buffer_desc);
                    staging_buffer->set_data(data, size, 0);

                    // @TODO: use KTX to generate mip maps: https://www.khronos.org/ktx/

                    device->submit_commands_immediate([&](const ICommandBuffer& cmd)
                    {
                        // Transition image layout to transfer dst
                        cmd.pipeline_barrier(this, TextureLayout::TransferDst, AccessMask::None,
                                             AccessMask::TransferWrite, PipelineStage::TopOfPipe,
                                             PipelineStage::Transfer);

                        cmd.copy_buffer_to_texture(staging_buffer.get(), this);

                        // Transition image layout to shader read only
                        cmd.pipeline_barrier(this, TextureLayout::ShaderReadOnly, AccessMask::TransferWrite,
                                             AccessMask::ShaderRead, PipelineStage::Transfer,
                                             PipelineStage::FragmentShader);
                    });
                }

                const math::uvec3& get_extent() const override { return extent; }

                Format get_format() const override { return format; }

                TextureLayout get_layout() const override { return layout; }

                TextureType get_type() const override { return type; }

                TextureViewType get_view_type() const override { return view_type; }

                TextureAspect get_aspect() const override { return aspect; }

                TextureUsage get_usage() const override { return usage; }

                SampleCount get_sample_count() const override { return sample_count; }

                u32 get_mip_levels() const override { return mip_levels; }

                u32 get_array_layers() const override { return array_layers; }

                const VkImage& get_image() const { return image; }

                const VkImageView& get_image_view() const { return image_view; }

                void set_new_layout(const VkImageLayout new_image_layout) { layout = vk_to_mag(new_image_layout); }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VmaAllocator allocator = nullptr;
                IDevice* device = nullptr;
                VkImage image = {};
                VkImageView image_view = {};
                VmaAllocation allocation = nullptr;
                math::uvec3 extent = {1, 1, 1};
                Format format = Format::B8G8R8A8_SRGB;
                TextureType type = TextureType::Texture2D;
                TextureViewType view_type = TextureViewType::Texture2D;
                TextureAspect aspect = TextureAspect::Color;
                TextureUsage usage = TextureUsage::ColorAttachment;
                TextureLayout layout = TextureLayout::Undefined;
                u32 mip_levels = 1;
                u32 array_layers = 1;
                SampleCount sample_count = SampleCount::e1;
        };

        class VulkanSwapchain : public ISwapchain
        {
            public:
                VulkanSwapchain(const ISwapchainDesc& desc, const vkb::DispatchTable* disp, const vkb::Device* device)
                    : disp(disp), device(device), present_mode(desc.desired_present_mode)
                {
                    recreate_swapchain(desc.desired_extent);
                }

                ~VulkanSwapchain() override
                {
                    vkb::destroy_swapchain(swapchain);
                    swapchain_textures.clear();
                }

                u32 get_current_image_index() const override { return current_image_index; }

                u32 get_image_count() const override { return swapchain.image_count; }

                math::uvec2 get_extent() const override { return vk_to_mag(swapchain.extent); }

                Format get_format() const override { return vk_to_mag(swapchain.image_format); }

                ITexture* get_texture(const u32 index) const override { return swapchain_textures[index].get(); }

                Result acquire_next_image(const ISemaphore* const signal_semaphore, const IFence* const fence) override
                {
                    VkSemaphore vk_sem = dynamic_cast<const VulkanSemaphore* const>(signal_semaphore)->get_semaphore();
                    VkFence vk_fen = nullptr;

                    if (fence != nullptr)
                    {
                        vk_fen = dynamic_cast<const VulkanFence* const>(fence)->get_fence();
                    }

                    const VkResult result =
                        disp->acquireNextImageKHR(swapchain, Timeout, vk_sem, vk_fen, &current_image_index);

                    return vk_to_mag(result);
                }

                void resize(const math::uvec2& extent) override { recreate_swapchain(extent); }

                const VkSwapchainKHR& get_swapchain() const { return swapchain.swapchain; }

            private:
                void recreate_swapchain(const math::uvec2& extent)
                {
                    vkb::SwapchainBuilder swapchain_builder{*device};
                    const auto swap_ret = swapchain_builder.set_old_swapchain(swapchain)
                                              .set_desired_extent(extent.x, extent.y)
                                              .set_desired_present_mode(mag_to_vk(present_mode))
                                              .add_fallback_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
                                              .add_fallback_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
                                              .add_fallback_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                              .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                              .build();

                    MAG_ASSERT(swap_ret, "{0} {1}", swap_ret.error().message(), std::to_string(swap_ret.vk_result()));

                    swapchain_textures.clear();

                    vkb::destroy_swapchain(swapchain);

                    swapchain = swap_ret.value();

                    const std::vector<VkImage>& swapchain_images = swapchain.get_images().value();
                    const std::vector<VkImageView>& swapchain_image_views = swapchain.get_image_views().value();

                    for (u32 i = 0; i < swapchain.image_count; i++)
                    {
                        auto* const texture =
                            new VulkanTexture(disp, nullptr, math::uvec3(vk_to_mag(swapchain.extent), 1),
                                              swapchain_images[i], swapchain_image_views[i]);

                        swapchain_textures.emplace_back(texture);
                    }
                }

                const vkb::DispatchTable* disp = nullptr;
                const vkb::Device* device = nullptr;
                vkb::Swapchain swapchain;
                PresentMode present_mode = PresentMode::Mailbox;
                u32 current_image_index = 0;
                std::vector<unique<VulkanTexture>> swapchain_textures;
        };

        class VulkanDescriptorPool : public IDescriptorPool
        {
            public:
                VulkanDescriptorPool(const IDescriptorPoolDesc& desc, const vkb::DispatchTable* disp) : disp(disp)
                {
                    std::vector<VkDescriptorPoolSize> pool_sizes;
                    for (const IDescriptorPoolSizeDesc& size_desc : desc.size_descs)
                    {
                        VkDescriptorPoolSize pool_size = {};
                        pool_size.type = mag_to_vk(size_desc.type);
                        pool_size.descriptorCount = size_desc.count;

                        pool_sizes.push_back(pool_size);
                    }

                    VkDescriptorPoolCreateInfo pool_info = {};
                    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    pool_info.poolSizeCount = pool_sizes.size();
                    pool_info.pPoolSizes = pool_sizes.data();
                    pool_info.maxSets = desc.max_sets;
                    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT |
                                      VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

                    vk_check(disp->createDescriptorPool(&pool_info, nullptr, &descriptor_pool),
                             "Failed to create descriptor pool");
                }

                ~VulkanDescriptorPool() override { disp->destroyDescriptorPool(descriptor_pool, nullptr); }

                const VkDescriptorPool& get_pool() const { return descriptor_pool; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkDescriptorPool descriptor_pool = {};
        };

        class VulkanDescriptorSetLayout : public IDescriptorSetLayout
        {
            public:
                VulkanDescriptorSetLayout(const IDescriptorSetLayoutDesc& desc, const vkb::DispatchTable* disp)
                    : disp(disp)
                {
                    std::vector<VkDescriptorSetLayoutBinding> bindings;
                    std::vector<VkDescriptorBindingFlags> flags;

                    for (const IDescriptorSetLayoutBindingDesc& binding_desc : desc.binding_descs)
                    {
                        VkDescriptorSetLayoutBinding binding = {};
                        binding.binding = binding_desc.binding;
                        binding.descriptorType = mag_to_vk(binding_desc.descriptor_type);
                        binding.descriptorCount = binding_desc.descriptor_count;
                        binding.stageFlags = mag_to_vk(binding_desc.stages);

                        VkDescriptorBindingFlags binding_flags =
                            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

                        if (binding_desc.variable_descriptor_count)
                        {
                            binding_flags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
                        }

                        bindings.push_back(binding);
                        flags.push_back(binding_flags);
                    }

                    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags = {};
                    binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                    binding_flags.bindingCount = bindings.size();
                    binding_flags.pBindingFlags = flags.data();

                    VkDescriptorSetLayoutCreateInfo layout_info = {};
                    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layout_info.bindingCount = bindings.size();
                    layout_info.pBindings = bindings.data();
                    layout_info.pNext = &binding_flags;
                    layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

                    vk_check(disp->createDescriptorSetLayout(&layout_info, nullptr, &descriptor_layout),
                             "Failed to create descriptor set layout");
                }

                ~VulkanDescriptorSetLayout() override { disp->destroyDescriptorSetLayout(descriptor_layout, nullptr); }

                const VkDescriptorSetLayout& get_layout() const { return descriptor_layout; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkDescriptorSetLayout descriptor_layout = {};
        };

        class VulkanDescriptorSet : public IDescriptorSet
        {
            public:
                VulkanDescriptorSet(const IDescriptorSetDesc& desc, const vkb::DispatchTable* disp)
                    : disp(disp),
                      parent_pool(dynamic_cast<const VulkanDescriptorPool* const>(desc.descriptor_pool)->get_pool())
                {
                    VkDescriptorSetLayout descriptor_layout =
                        dynamic_cast<const VulkanDescriptorSetLayout* const>(desc.descriptor_layout)->get_layout();

                    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_info = {};
                    variable_count_info.sType =
                        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variable_count_info.descriptorSetCount = 1;
                    variable_count_info.pDescriptorCounts = &desc.max_variable_descriptor_count;

                    VkDescriptorSetAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    alloc_info.descriptorPool = parent_pool;
                    alloc_info.descriptorSetCount = 1;
                    alloc_info.pSetLayouts = &descriptor_layout;
                    alloc_info.pNext = &variable_count_info;

                    vk_check(disp->allocateDescriptorSets(&alloc_info, &descriptor_set),
                             "Failed to allocate descriptor sets");
                }

                ~VulkanDescriptorSet() override { disp->freeDescriptorSets(parent_pool, 1, &descriptor_set); }

                void update(const IBuffer* const buffer, const u32 binding, const u32 array_element,
                            const DescriptorType descriptor_type, const u64 offset) const override
                {
                    std::vector<VkWriteDescriptorSet> descriptor_writes;

                    VkDescriptorBufferInfo buffer_info = {};
                    buffer_info.buffer = dynamic_cast<const VulkanBuffer* const>(buffer)->get_buffer();
                    buffer_info.offset = offset;
                    buffer_info.range = buffer->get_size();

                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptor_set;
                    write.dstBinding = binding;
                    write.dstArrayElement = array_element;
                    write.descriptorType = mag_to_vk(descriptor_type);
                    write.descriptorCount = 1;
                    write.pBufferInfo = &buffer_info;

                    descriptor_writes.push_back(write);

                    disp->updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
                }

                void update(const ITexture* const texture, const ISampler* const sampler, const u32 binding,
                            const u32 array_element, const DescriptorType descriptor_type) const override
                {
                    std::vector<VkWriteDescriptorSet> descriptor_writes;

                    VkDescriptorImageInfo image_info = {};
                    image_info.imageLayout = mag_to_vk(texture->get_layout());
                    image_info.imageView = dynamic_cast<const VulkanTexture* const>(texture)->get_image_view();
                    image_info.sampler = dynamic_cast<const VulkanSampler* const>(sampler)->get_sampler();

                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptor_set;
                    write.dstBinding = binding;
                    write.dstArrayElement = array_element;
                    write.descriptorType = mag_to_vk(descriptor_type);
                    write.descriptorCount = 1;
                    write.pImageInfo = &image_info;

                    descriptor_writes.push_back(write);

                    disp->updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
                }

                const VkDescriptorSet& get_descriptor_set() const { return descriptor_set; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkDescriptorSet descriptor_set = {};
                VkDescriptorPool parent_pool = {};
        };

        class VulkanGraphicsPipeline : public IGraphicsPipeline
        {
            public:
                VulkanGraphicsPipeline(const IGraphicsPipelineDesc& desc, const vkb::DispatchTable* disp) : disp(disp)
                {
                    const u32 shader_module_count = desc.shader_modules.size();

                    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(shader_module_count);
                    std::vector<VkShaderModule> shader_modules(shader_module_count);

                    for (u32 i = 0; i < shader_module_count; i++)
                    {
                        const IShaderModuleDesc& shader_module_desc = desc.shader_modules[i];

                        VkShaderModuleCreateInfo shader_module_info = {};
                        shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        shader_module_info.codeSize = shader_module_desc.code.size();

                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                        shader_module_info.pCode = reinterpret_cast<const u32* const>(shader_module_desc.code.data());

                        shader_modules[i] = {};

                        vk_check(disp->createShaderModule(&shader_module_info, nullptr, &shader_modules[i]),
                                 "Failed to create shader module");

                        shader_stages[i] = {
                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                            .pNext = nullptr,
                            .flags = 0,
                            .stage = mag_to_vk_bits(shader_module_desc.stage),
                            .module = shader_modules[i],
                            .pName = "main",
                            .pSpecializationInfo = nullptr,
                        };
                    }

                    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
                    for (const IDescriptorSetLayout* const descriptor_layout : desc.descriptor_layouts)
                    {
                        VkDescriptorSetLayout descriptor_set_layout =
                            dynamic_cast<const VulkanDescriptorSetLayout* const>(descriptor_layout)->get_layout();

                        descriptor_set_layouts.push_back(descriptor_set_layout);
                    }

                    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
                    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                    vertex_input_info.vertexAttributeDescriptionCount = desc.vertex_attribute_descs.size();
                    vertex_input_info.vertexBindingDescriptionCount = desc.vertex_binding_descs.size();

                    std::vector<VkVertexInputAttributeDescription> vertex_attribute_infos;
                    std::vector<VkVertexInputBindingDescription> vertex_binding_infos;

                    for (const IVertexAttributeDesc& vertex_attribute_desc : desc.vertex_attribute_descs)
                    {
                        VkVertexInputAttributeDescription vertex_attribute_info = {};

                        vertex_attribute_info.format = mag_to_vk(vertex_attribute_desc.format);
                        vertex_attribute_info.binding = vertex_attribute_desc.binding;
                        vertex_attribute_info.location = vertex_attribute_desc.location;
                        vertex_attribute_info.offset = vertex_attribute_desc.offset;

                        vertex_attribute_infos.push_back(vertex_attribute_info);
                    }

                    for (const IVertexBindingDesc& vertex_binding_desc : desc.vertex_binding_descs)
                    {
                        VkVertexInputBindingDescription vertex_binding_info = {};

                        vertex_binding_info.inputRate = mag_to_vk(vertex_binding_desc.input_rate);
                        vertex_binding_info.binding = vertex_binding_desc.binding;
                        vertex_binding_info.stride = vertex_binding_desc.stride;

                        vertex_binding_infos.push_back(vertex_binding_info);
                    }

                    vertex_input_info.pVertexAttributeDescriptions = vertex_attribute_infos.data();
                    vertex_input_info.pVertexBindingDescriptions = vertex_binding_infos.data();

                    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
                    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                    input_assembly.topology = mag_to_vk(desc.primitive_topology);
                    input_assembly.primitiveRestartEnable = VK_FALSE;

                    VkViewport viewport = {};
                    viewport.x = 0.0F;
                    viewport.y = 0.0F;
                    viewport.width = static_cast<f32>(desc.extent.x);
                    viewport.height = static_cast<f32>(desc.extent.y);
                    viewport.minDepth = 0.0F;
                    viewport.maxDepth = 1.0F;

                    VkRect2D scissor = {};
                    scissor.offset = {.x = 0, .y = 0};
                    scissor.extent = mag_to_vk(desc.extent);

                    VkPipelineViewportStateCreateInfo viewport_state = {};
                    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                    viewport_state.viewportCount = 1;
                    viewport_state.pViewports = &viewport;
                    viewport_state.scissorCount = 1;
                    viewport_state.pScissors = &scissor;

                    VkPipelineRasterizationStateCreateInfo rasterizer = {};
                    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                    rasterizer.depthClampEnable = VK_FALSE;
                    rasterizer.rasterizerDiscardEnable = VK_FALSE;
                    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                    rasterizer.lineWidth = 1.0F;
                    rasterizer.cullMode = VK_CULL_MODE_NONE;
                    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
                    rasterizer.depthBiasEnable = VK_FALSE;

                    const VkPipelineMultisampleStateCreateInfo multisampling = {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = 0,
                        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                        .sampleShadingEnable = VK_FALSE,
                        .minSampleShading = 0.0F,
                        .pSampleMask = nullptr,
                        .alphaToCoverageEnable = VK_FALSE,
                        .alphaToOneEnable = VK_FALSE,
                    };

                    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
                    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                    color_blend_attachment.blendEnable = static_cast<VkBool32>(desc.color_blend.blend_enable);
                    color_blend_attachment.colorBlendOp = mag_to_vk(desc.color_blend.color_blend_op);
                    color_blend_attachment.srcColorBlendFactor = mag_to_vk(desc.color_blend.src_color_blend_factor);
                    color_blend_attachment.dstColorBlendFactor = mag_to_vk(desc.color_blend.dst_color_blend_factor);
                    color_blend_attachment.alphaBlendOp = mag_to_vk(desc.color_blend.alpha_blend_op);
                    color_blend_attachment.srcAlphaBlendFactor = mag_to_vk(desc.color_blend.src_alpha_blend_factor);
                    color_blend_attachment.dstAlphaBlendFactor = mag_to_vk(desc.color_blend.dst_alpha_blend_factor);

                    VkPipelineColorBlendStateCreateInfo color_blending = {};
                    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                    color_blending.logicOpEnable = VK_FALSE;
                    color_blending.logicOp = VK_LOGIC_OP_COPY;
                    color_blending.attachmentCount = 1;
                    color_blending.pAttachments = &color_blend_attachment;

                    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
                    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                    pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
                    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
                    pipeline_layout_info.pushConstantRangeCount = 0;

                    if (disp->createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create pipeline layout");
                    }

                    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

                    VkPipelineDynamicStateCreateInfo dynamic_info = {};
                    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                    dynamic_info.dynamicStateCount = static_cast<u32>(dynamic_states.size());
                    dynamic_info.pDynamicStates = dynamic_states.data();

                    const VkFormat swapchain_format = mag_to_vk(desc.color_attachment_format);

                    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
                    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
                    pipeline_rendering_create_info.colorAttachmentCount = 1;
                    pipeline_rendering_create_info.depthAttachmentFormat = mag_to_vk(desc.depth_attachment_format);
                    pipeline_rendering_create_info.pColorAttachmentFormats = &swapchain_format;

                    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
                    depth_stencil_create_info.depthTestEnable = 1U;
                    depth_stencil_create_info.depthWriteEnable = 1U;
                    depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                    depth_stencil_create_info.minDepthBounds = 0.0F;
                    depth_stencil_create_info.maxDepthBounds = 1.0F;

                    VkGraphicsPipelineCreateInfo pipeline_info = {};
                    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                    pipeline_info.pDepthStencilState = &depth_stencil_create_info;
                    pipeline_info.stageCount = shader_module_count;
                    pipeline_info.pStages = shader_stages.data();
                    pipeline_info.pVertexInputState = &vertex_input_info;
                    pipeline_info.pInputAssemblyState = &input_assembly;
                    pipeline_info.pViewportState = &viewport_state;
                    pipeline_info.pRasterizationState = &rasterizer;
                    pipeline_info.pMultisampleState = &multisampling;
                    pipeline_info.pColorBlendState = &color_blending;
                    pipeline_info.pDynamicState = &dynamic_info;
                    pipeline_info.layout = pipeline_layout;
                    pipeline_info.renderPass = nullptr;
                    pipeline_info.subpass = 0;
                    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
                    pipeline_info.pNext = &pipeline_rendering_create_info;

                    if (disp->createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) !=
                        VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create pipeline");
                    }

                    for (u32 i = 0; i < shader_module_count; i++)
                    {
                        disp->destroyShaderModule(shader_modules[i], nullptr);
                    }
                }

                ~VulkanGraphicsPipeline() override
                {
                    disp->destroyPipeline(pipeline, nullptr);
                    disp->destroyPipelineLayout(pipeline_layout, nullptr);
                }

                const VkPipeline& get_pipeline() const { return pipeline; }

                const VkPipelineLayout& get_pipeline_layout() const { return pipeline_layout; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkPipelineLayout pipeline_layout = {};
                VkPipeline pipeline = {};
        };

        class VulkanRenderingAttachment : public IRenderingAttachment
        {
            public:
                explicit VulkanRenderingAttachment(const IRenderingAttachmentDesc& desc)
                {
                    VkClearValue clear_value = {};

                    // Choose clear value based on the attachment type
                    if (desc.type == RenderingAttachmentType::Color)
                    {
                        clear_value.color = mag_to_vk(desc.clear_color);
                    }

                    else
                    {
                        clear_value.depthStencil.depth = desc.clear_depth;
                        clear_value.depthStencil.stencil = desc.clear_stencil;
                    }

                    rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
                    rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
                    rendering_attachment_info.imageView =
                        dynamic_cast<const VulkanTexture* const>(desc.texture)->get_image_view();
                    rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    rendering_attachment_info.clearValue = clear_value;
                }

                ~VulkanRenderingAttachment() override = default;

                math::vec4 get_clear_color() const override
                {
                    return vk_to_mag(rendering_attachment_info.clearValue.color);
                }

                f32 get_clear_depth() const override { return rendering_attachment_info.clearValue.depthStencil.depth; }

                u32 get_clear_stencil() const override
                {
                    return rendering_attachment_info.clearValue.depthStencil.stencil;
                }

                const VkRenderingAttachmentInfoKHR& get_attachment_info() const { return rendering_attachment_info; }

            private:
                VkRenderingAttachmentInfoKHR rendering_attachment_info = {};
        };

        class VulkanRenderPass : public IRenderPass
        {
            public:
                explicit VulkanRenderPass(const IRenderPassDesc& desc)
                {
                    VkRect2D render_area = {};
                    render_area.extent = mag_to_vk(desc.extent);
                    render_area.offset = mag_to_vk(desc.offset);

                    for (const IRenderingAttachment* const color_attachment : desc.color_attachments)
                    {
                        const VkRenderingAttachmentInfo attachment_info =
                            dynamic_cast<const VulkanRenderingAttachment* const>(color_attachment)
                                ->get_attachment_info();

                        color_attachments.push_back(attachment_info);
                    }

                    render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
                    render_info.renderArea = render_area;
                    render_info.layerCount = 1;

                    if (!color_attachments.empty())
                    {
                        render_info.colorAttachmentCount = color_attachments.size();
                        render_info.pColorAttachments = color_attachments.data();
                    }

                    if (desc.depth_attachment != nullptr)
                    {
                        depth_attachment = dynamic_cast<const VulkanRenderingAttachment* const>(desc.depth_attachment)
                                               ->get_attachment_info();

                        render_info.pDepthAttachment = &depth_attachment;
                    }
                }

                ~VulkanRenderPass() override = default;

                math::ivec2 get_offset() const override { return vk_to_mag(render_info.renderArea.offset); }

                math::uvec2 get_extent() const override { return vk_to_mag(render_info.renderArea.extent); }

                const VkRenderingInfoKHR& get_rendering_info() const { return render_info; }

            private:
                VkRenderingInfoKHR render_info = {};
                std::vector<VkRenderingAttachmentInfo> color_attachments;
                VkRenderingAttachmentInfo depth_attachment = {};
        };

        class VulkanCommandPool : public ICommandPool
        {
            public:
                VulkanCommandPool(const ICommandPoolDesc& desc, const vkb::DispatchTable* disp,
                                  const vkb::Device& device)
                    : disp(disp)
                {
                    VkCommandPoolCreateInfo pool_info = {};
                    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    pool_info.queueFamilyIndex = device.get_queue_index(mag_to_vk(desc.queue_type)).value();

                    vk_check(disp->createCommandPool(&pool_info, nullptr, &pool), "Failed to create command pool");
                }

                ~VulkanCommandPool() override { disp->destroyCommandPool(pool, nullptr); }

                void reset() const override { disp->resetCommandPool(pool, 0); }

                const VkCommandPool& get_pool() const { return pool; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkCommandPool pool = {};
        };

        class VulkanCommandBuffer : public ICommandBuffer
        {
            public:
                VulkanCommandBuffer(const ICommandBufferDesc& desc, const vkb::DispatchTable* disp)
                    : disp(disp), command_pool(dynamic_cast<const VulkanCommandPool* const>(desc.command_pool))
                {
                    VkCommandBufferAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    alloc_info.commandPool = command_pool->get_pool();
                    alloc_info.level = mag_to_vk(desc.command_buffer_level);
                    alloc_info.commandBufferCount = 1;

                    vk_check(disp->allocateCommandBuffers(&alloc_info, &command_buffer),
                             "Failed to allocate command buffer");
                }

                ~VulkanCommandBuffer() override
                {
                    disp->freeCommandBuffers(command_pool->get_pool(), 1, &command_buffer);
                }

                void begin_recording() const override
                {
                    VkCommandBufferBeginInfo begin_info = {};
                    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                    vk_check(disp->beginCommandBuffer(command_buffer, &begin_info),
                             "Failed to begin command buffer recording");
                }

                void end_recording() const override
                {
                    vk_check(disp->endCommandBuffer(command_buffer), "Failed to record command buffer");
                }

                void reset() const override { disp->resetCommandBuffer(command_buffer, 0); }

                void set_viewport(const math::vec2& extent, const math::vec2& offset, const f32 min_depth,
                                  const f32 max_depth) const override
                {
                    VkViewport viewport = {};
                    viewport.width = extent.x;
                    viewport.height = extent.y;
                    viewport.x = offset.x;
                    viewport.y = offset.y;
                    viewport.minDepth = min_depth;
                    viewport.maxDepth = max_depth;

                    disp->cmdSetViewport(command_buffer, 0, 1, &viewport);
                }

                void set_scissor(const math::uvec2& extent, const math::ivec2& offset) const override
                {
                    VkRect2D scissor = {};
                    scissor.extent = mag_to_vk(extent);
                    scissor.offset = mag_to_vk(offset);

                    disp->cmdSetScissor(command_buffer, 0, 1, &scissor);
                }

                void begin_rendering(const IRenderPass* render_pass) const override
                {
                    const VkRenderingInfo* rendering_info =
                        &dynamic_cast<const VulkanRenderPass* const>(render_pass)->get_rendering_info();

                    disp->cmdBeginRendering(command_buffer, rendering_info);
                }

                void end_rendering() const override { disp->cmdEndRendering(command_buffer); }

                void bind_pipeline(const IGraphicsPipeline* const pipeline) const override
                {
                    disp->cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          dynamic_cast<const VulkanGraphicsPipeline* const>(pipeline)->get_pipeline());
                }

                void bind_descriptor(const IGraphicsPipeline* const pipeline,
                                     const IDescriptorSet* const descriptor) const override
                {
                    VkPipelineLayout pipeline_layout =
                        dynamic_cast<const VulkanGraphicsPipeline* const>(pipeline)->get_pipeline_layout();

                    VkDescriptorSet descriptor_set =
                        dynamic_cast<const VulkanDescriptorSet* const>(descriptor)->get_descriptor_set();

                    disp->cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                                &descriptor_set, 0, nullptr);
                }

                void bind_vertex_buffers(const u32 first_binding, const u32 binding_count,
                                         const std::vector<const IBuffer*>& buffers,
                                         const std::vector<u64>& offsets) const override
                {
                    std::vector<VkBuffer> vk_buffers(buffers.size());

                    for (u64 i = 0; i < buffers.size(); i++)
                    {
                        vk_buffers[i] = dynamic_cast<const VulkanBuffer* const>(buffers[i])->get_buffer();
                    }

                    disp->cmdBindVertexBuffers(command_buffer, first_binding, binding_count, vk_buffers.data(),
                                               offsets.data());
                }

                void bind_index_buffer(const IBuffer* const buffer, const u64 offset) const override
                {
                    VkBuffer vk_buffer = dynamic_cast<const VulkanBuffer* const>(buffer)->get_buffer();

                    disp->cmdBindIndexBuffer(command_buffer, vk_buffer, offset, VK_INDEX_TYPE_UINT32);
                }

                void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                          const u32 first_instance) const override
                {
                    disp->cmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
                }

                void draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index,
                                  const i32 vertex_offset, const u32 first_instance) const override
                {
                    disp->cmdDrawIndexed(command_buffer, index_count, instance_count, first_index, vertex_offset,
                                         first_instance);
                }

                void draw_indexed_indirect(const IBuffer* const buffer, const u64 offset, const u32 draw_count,
                                           const u32 stride) const override
                {
                    VkBuffer vk_buffer = dynamic_cast<const VulkanBuffer* const>(buffer)->get_buffer();

                    disp->cmdDrawIndexedIndirect(command_buffer, vk_buffer, offset, draw_count, stride);
                }

                void pipeline_barrier(ITexture* const texture, const TextureLayout new_layout,
                                      const AccessMask src_access_mask, const AccessMask dst_access_mask,
                                      const PipelineStage src_stage_mask,
                                      const PipelineStage dst_stage_mask) const override
                {
                    VkImageMemoryBarrier image_memory_barrier = {};
                    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_memory_barrier.srcAccessMask = mag_to_vk(src_access_mask);
                    image_memory_barrier.dstAccessMask = mag_to_vk(dst_access_mask);
                    image_memory_barrier.oldLayout = mag_to_vk(texture->get_layout());
                    image_memory_barrier.newLayout = mag_to_vk(new_layout);
                    image_memory_barrier.image = dynamic_cast<const VulkanTexture* const>(texture)->get_image();
                    image_memory_barrier.subresourceRange = {
                        .aspectMask = mag_to_vk(texture->get_aspect()),
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    };

                    disp->cmdPipelineBarrier(command_buffer, mag_to_vk(src_stage_mask), mag_to_vk(dst_stage_mask),
                                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                                             &image_memory_barrier);

                    dynamic_cast<VulkanTexture*>(texture)->set_new_layout(mag_to_vk(new_layout));
                }

                void blit_texture(const ITexture* const src_texture, const ITexture* const dst_texture,
                                  const Filter filter) const override
                {
                    const auto* const vk_src = dynamic_cast<const VulkanTexture*>(src_texture);
                    const auto* const vk_dst = dynamic_cast<const VulkanTexture*>(dst_texture);

                    VkImageBlit image_blit = {};

                    // Src
                    const math::uvec3& src_extent = vk_src->get_extent();

                    image_blit.srcOffsets[1].x = static_cast<i32>(src_extent.x);
                    image_blit.srcOffsets[1].y = static_cast<i32>(src_extent.y);
                    image_blit.srcOffsets[1].z = static_cast<i32>(src_extent.z);

                    image_blit.srcSubresource.layerCount = src_texture->get_array_layers();
                    image_blit.srcSubresource.aspectMask = mag_to_vk(vk_src->get_aspect());
                    image_blit.srcSubresource.baseArrayLayer = 0;
                    image_blit.srcSubresource.mipLevel = 0;

                    // Dst
                    const math::uvec3& dst_extent = vk_dst->get_extent();

                    image_blit.dstOffsets[1].x = static_cast<i32>(dst_extent.x);
                    image_blit.dstOffsets[1].y = static_cast<i32>(dst_extent.y);
                    image_blit.dstOffsets[1].z = static_cast<i32>(dst_extent.z);

                    image_blit.dstSubresource.layerCount = dst_texture->get_array_layers();
                    image_blit.dstSubresource.aspectMask = mag_to_vk(vk_dst->get_aspect());
                    image_blit.dstSubresource.baseArrayLayer = 0;
                    image_blit.dstSubresource.mipLevel = 0;

                    disp->cmdBlitImage(command_buffer, vk_src->get_image(), mag_to_vk(vk_src->get_layout()),
                                       vk_dst->get_image(), mag_to_vk(vk_dst->get_layout()), 1, &image_blit,
                                       mag_to_vk(filter));
                }

                void copy_texture(const ITexture* const src_texture, const ITexture* const dst_texture) const override
                {
                    const auto* const vk_src = dynamic_cast<const VulkanTexture*>(src_texture);
                    const auto* const vk_dst = dynamic_cast<const VulkanTexture*>(dst_texture);

                    const math::uvec3& extent = math::min(vk_src->get_extent(), vk_dst->get_extent());

                    VkImageCopy image_copy = {};
                    image_copy.extent = mag_to_vk(extent);

                    image_copy.srcSubresource.layerCount = src_texture->get_array_layers();
                    image_copy.srcSubresource.aspectMask = mag_to_vk(vk_src->get_aspect());
                    image_copy.srcOffset = {};

                    image_copy.dstSubresource.layerCount = dst_texture->get_array_layers();
                    image_copy.dstSubresource.aspectMask = mag_to_vk(vk_dst->get_aspect());
                    image_copy.dstOffset = {};

                    disp->cmdCopyImage(command_buffer, vk_src->get_image(), mag_to_vk(vk_src->get_layout()),
                                       vk_dst->get_image(), mag_to_vk(vk_dst->get_layout()), 1, &image_copy);
                }

                void copy_buffer_to_texture(const IBuffer* const buffer, const ITexture* const texture) const override
                {
                    const auto* const vk_buffer = dynamic_cast<const VulkanBuffer*>(buffer);
                    const auto* const vk_texture = dynamic_cast<const VulkanTexture*>(texture);

                    VkBufferImageCopy buffer_image_copy = {};
                    buffer_image_copy.bufferImageHeight = texture->get_extent().y;
                    buffer_image_copy.bufferRowLength = texture->get_extent().x;
                    buffer_image_copy.imageExtent = mag_to_vk(texture->get_extent());
                    buffer_image_copy.imageSubresource.aspectMask = mag_to_vk(texture->get_aspect());
                    buffer_image_copy.imageSubresource.baseArrayLayer = 0;
                    buffer_image_copy.imageSubresource.layerCount = texture->get_array_layers();
                    buffer_image_copy.imageSubresource.mipLevel = 0;

                    disp->cmdCopyBufferToImage(command_buffer, vk_buffer->get_buffer(), vk_texture->get_image(),
                                               mag_to_vk(vk_texture->get_layout()), 1, &buffer_image_copy);
                }

                const VkCommandBuffer& get_command_buffer() const { return command_buffer; }

            private:
                const vkb::DispatchTable* disp = nullptr;
                const VulkanCommandPool* command_pool;
                VkCommandBuffer command_buffer = {};
        };

        class VulkanQueue : public IQueue
        {
            public:
                VulkanQueue(const IQueueDesc& desc, const vkb::DispatchTable* disp, const vkb::Device& device)
                    : disp(disp)
                {
                    const vkb::Result<VkQueue> queue_ret = device.get_queue(mag_to_vk(desc.queue_type));

                    MAG_ASSERT(queue_ret, "{}", queue_ret.error().message());

                    queue = queue_ret.value();
                }

                ~VulkanQueue() override = default;

                void submit(const ISemaphore* const wait_semaphore, const ISemaphore* const signal_semaphore,
                            const IFence* const fence, const ICommandBuffer* const command_buffer) const override
                {
                    VkSubmitInfo submit_info = {};
                    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                    std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                    submit_info.pWaitDstStageMask = wait_stages.data();

                    if (wait_semaphore != nullptr)
                    {
                        submit_info.waitSemaphoreCount = 1;
                        submit_info.pWaitSemaphores =
                            &dynamic_cast<const VulkanSemaphore* const>(wait_semaphore)->get_semaphore();
                    }

                    if (signal_semaphore != nullptr)
                    {
                        submit_info.signalSemaphoreCount = 1;
                        submit_info.pSignalSemaphores =
                            &dynamic_cast<const VulkanSemaphore* const>(signal_semaphore)->get_semaphore();
                    }

                    if (command_buffer != nullptr)
                    {
                        submit_info.commandBufferCount = 1;
                        submit_info.pCommandBuffers =
                            &dynamic_cast<const VulkanCommandBuffer* const>(command_buffer)->get_command_buffer();
                    }

                    fence->reset();

                    vk_check(disp->queueSubmit(queue, 1, &submit_info,
                                               dynamic_cast<const VulkanFence* const>(fence)->get_fence()),
                             "Failed to submit draw command buffer");
                }

                Result present(const ISwapchain* const swapchain, const ISemaphore* const wait_semaphore) const override
                {
                    const u32 image_index =
                        dynamic_cast<const VulkanSwapchain* const>(swapchain)->get_current_image_index();

                    VkPresentInfoKHR present_info = {};
                    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                    present_info.waitSemaphoreCount = 1;
                    present_info.pWaitSemaphores =
                        &dynamic_cast<const VulkanSemaphore* const>(wait_semaphore)->get_semaphore();
                    present_info.swapchainCount = 1;
                    present_info.pSwapchains = &dynamic_cast<const VulkanSwapchain* const>(swapchain)->get_swapchain();
                    present_info.pImageIndices = &image_index;

                    const VkResult result = disp->queuePresentKHR(queue, &present_info);

                    return vk_to_mag(result);
                }

            private:
                const vkb::DispatchTable* disp = nullptr;
                VkQueue queue;
        };

        class VulkanDevice : public IDevice
        {
            public:
                VulkanDevice()
                {
                    const u32 vulkan_major_version = 1;
                    const u32 vulkan_minor_version = 3;
                    const u32 vulkan_patch_version = 0;

                    // Device
                    // -------------------------------------------------------------------------------------------------
                    vkb::InstanceBuilder instance_builder;
                    const vkb::Result<vkb::Instance> instance_ret =
                        instance_builder

#if MAG_CONFIG_DEBUG
                            .set_debug_callback(
                                [](VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                   VkDebugUtilsMessageTypeFlagsEXT message_type,
                                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                   void* user_data) -> VkBool32
                    {
                        (void)message_type;
                        (void)user_data;

                        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                        {
                            LOG_WARNING("{0}\n", callback_data->pMessage);
                        }

                        else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                        {
                            LOG_ERROR("{0}\n", callback_data->pMessage);
                            DEBUG_BREAK();
                        }

                        else
                        {
                            LOG_INFO("{0}\n", callback_data->pMessage);
                        }

                        return VK_FALSE;
                    })
                            .request_validation_layers()

                            .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT)
                            .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
                    // .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)
                    // .add_validation_feature_enable(
                    //     VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT)
#endif
                            .require_api_version(vulkan_major_version, vulkan_minor_version, vulkan_patch_version)
                            .build();

                    MAG_ASSERT(instance_ret, "{}", instance_ret.error().message());

                    instance = instance_ret.value();
                    inst_disp = instance.make_table();

                    window::create_surface(static_cast<void*>(&instance.instance), static_cast<void*>(&surface));

                    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = {};
                    dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
                    dynamic_rendering_features.dynamicRendering = 1U;

                    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {};
                    descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
                    descriptor_indexing_features.descriptorBindingPartiallyBound = 1U;
                    descriptor_indexing_features.descriptorBindingVariableDescriptorCount = 1U;
                    descriptor_indexing_features.descriptorBindingUniformBufferUpdateAfterBind = 1U;
                    descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind = 1U;
                    descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind = 1U;
                    descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = 1U;
                    descriptor_indexing_features.shaderStorageBufferArrayNonUniformIndexing = 1U;
                    descriptor_indexing_features.shaderUniformBufferArrayNonUniformIndexing = 1U;
                    descriptor_indexing_features.runtimeDescriptorArray = 1U;

                    vkb::PhysicalDeviceSelector phys_device_selector(instance);
                    const vkb::Result<vkb::PhysicalDevice> phys_device_ret =
                        phys_device_selector.set_minimum_version(vulkan_major_version, vulkan_minor_version)
                            .set_surface(surface)
                            .add_required_extension_features(descriptor_indexing_features)
                            .add_required_extension_features(dynamic_rendering_features)
                            .select();

                    MAG_ASSERT(phys_device_ret, "{}", phys_device_ret.error().message());

                    const vkb::PhysicalDevice& physical_device = phys_device_ret.value();
                    const vkb::DeviceBuilder device_builder{physical_device};
                    const vkb::Result<vkb::Device> device_ret = device_builder.build();

                    MAG_ASSERT(device_ret, "{}", device_ret.error().message());

                    device = device_ret.value();

                    disp = device.make_table();

                    VmaAllocatorCreateInfo allocator_create_info = {};
                    allocator_create_info.physicalDevice = physical_device.physical_device;
                    allocator_create_info.device = device.device;
                    allocator_create_info.instance = instance.instance;
                    allocator_create_info.vulkanApiVersion = instance.api_version;
                    // allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

                    vk_check(vmaCreateAllocator(&allocator_create_info, &allocator),
                             "Failed to create memory allocator");

                    // Immediate submission resources

                    ICommandPoolDesc command_pool_desc = {};
                    command_pool_desc.queue_type = QueueType::Graphics;
                    immediate_command_pool = create_unique<VulkanCommandPool>(command_pool_desc, &disp, device);

                    ICommandBufferDesc command_buffer_desc = {};
                    command_buffer_desc.command_buffer_level = CommandBufferLevel::Primary;
                    command_buffer_desc.command_pool = immediate_command_pool.get();
                    immediate_command_buffer = create_unique<VulkanCommandBuffer>(command_buffer_desc, &disp);

                    IQueueDesc queue_desc = {};
                    queue_desc.queue_type = QueueType::Graphics;
                    immediate_queue = create_unique<VulkanQueue>(queue_desc, &disp, device);

                    const IFenceDesc fence_desc = {};
                    immediate_fence = create_unique<VulkanFence>(fence_desc, &disp);
                }

                ~VulkanDevice() override
                {
                    disp.deviceWaitIdle();

                    immediate_fence = nullptr;
                    immediate_queue = nullptr;
                    immediate_command_buffer = nullptr;
                    immediate_command_pool = nullptr;

                    vmaDestroyAllocator(allocator);

                    vkb::destroy_device(device);
                    vkb::destroy_surface(instance, surface);
                    vkb::destroy_instance(instance);
                }

                void wait_idle() const override { disp.deviceWaitIdle(); }

                void submit_commands_immediate(const std::function<void(ICommandBuffer& cmd)>& function) const override
                {
                    const unique<ICommandBuffer>& cmd = immediate_command_buffer;

                    cmd->begin_recording();
                    function(*cmd);  // execute the function
                    cmd->end_recording();

                    immediate_queue->submit(nullptr, nullptr, immediate_fence.get(), immediate_command_buffer.get());
                    immediate_fence->wait();

                    (*immediate_fence).reset();
                    (*immediate_command_pool).reset();
                }

                unique<ISemaphore> create_semaphore(const ISemaphoreDesc& desc) const override
                {
                    return create_unique<VulkanSemaphore>(desc, &disp);
                }

                unique<IFence> create_fence(const IFenceDesc& desc) const override
                {
                    return create_unique<VulkanFence>(desc, &disp);
                }

                unique<ISwapchain> create_swapchain(const ISwapchainDesc& desc) const override
                {
                    return create_unique<VulkanSwapchain>(desc, &disp, &device);
                }

                unique<IQueue> create_queue(const IQueueDesc& desc) const override
                {
                    return create_unique<VulkanQueue>(desc, &disp, device);
                }

                unique<IGraphicsPipeline> create_graphics_pipeline(const IGraphicsPipelineDesc& desc) const override
                {
                    return create_unique<VulkanGraphicsPipeline>(desc, &disp);
                }

                unique<ICommandPool> create_command_pool(const ICommandPoolDesc& desc) const override
                {
                    return create_unique<VulkanCommandPool>(desc, &disp, device);
                }

                unique<ICommandBuffer> create_command_buffer(const ICommandBufferDesc& desc) const override
                {
                    return create_unique<VulkanCommandBuffer>(desc, &disp);
                }

                unique<IRenderingAttachment> create_render_attachment(
                    const IRenderingAttachmentDesc& desc) const override
                {
                    return create_unique<VulkanRenderingAttachment>(desc);
                }

                unique<IRenderPass> create_render_pass(const IRenderPassDesc& desc) const override
                {
                    return create_unique<VulkanRenderPass>(desc);
                }

                unique<ITexture> create_texture(const ITextureDesc& desc) override
                {
                    return create_unique<VulkanTexture>(desc, &disp, this, allocator);
                }

                unique<IBuffer> create_buffer(const IBufferDesc& desc) const override
                {
                    return create_unique<VulkanBuffer>(desc, allocator);
                }

                unique<IDescriptorPool> create_descriptor_pool(const IDescriptorPoolDesc& desc) const override
                {
                    return create_unique<VulkanDescriptorPool>(desc, &disp);
                }

                unique<IDescriptorSetLayout> create_descriptor_set_layout(
                    const IDescriptorSetLayoutDesc& desc) const override
                {
                    return create_unique<VulkanDescriptorSetLayout>(desc, &disp);
                }

                unique<IDescriptorSet> create_descriptor_set(const IDescriptorSetDesc& desc) const override
                {
                    return create_unique<VulkanDescriptorSet>(desc, &disp);
                }

                unique<ISampler> create_sampler(const ISamplerDesc& desc) const override
                {
                    return create_unique<VulkanSampler>(desc, &disp);
                }

                DescriptorLimits get_descriptor_limits() const override
                {
                    VkPhysicalDeviceProperties2 properties = {};
                    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

                    inst_disp.getPhysicalDeviceProperties2(device.physical_device, &properties);

                    DescriptorLimits limits = {};

                    limits.max_per_stage_combined_image_samplers =
                        properties.properties.limits.maxPerStageDescriptorSamplers;

                    limits.max_per_stage_storage_buffers =
                        properties.properties.limits.maxPerStageDescriptorStorageBuffers;

                    limits.max_per_stage_uniform_buffers =
                        properties.properties.limits.maxPerStageDescriptorUniformBuffers;

                    return limits;
                }

            private:
                vkb::Instance instance;
                vkb::Device device;
                vkb::InstanceDispatchTable inst_disp;
                vkb::DispatchTable disp;
                VkSurfaceKHR surface = {};
                VmaAllocator allocator = {};

                unique<ICommandBuffer> immediate_command_buffer;
                unique<ICommandPool> immediate_command_pool;
                unique<IQueue> immediate_queue;
                unique<IFence> immediate_fence;
        };

        unique<IDevice> create_device() { return create_unique<VulkanDevice>(); }
    };  // namespace gfx
};  // namespace mag
