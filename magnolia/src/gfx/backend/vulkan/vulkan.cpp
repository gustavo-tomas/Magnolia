#include "gfx/backend/backend.hpp"

#define MAG_CONFIG_GFX_VULKAN 1

#if MAG_CONFIG_GFX_VULKAN

    #include <vulkan/vulkan.h>

    #include <string>
    #include <vector>

    #include "VkBootstrap.h"
    #include "VkBootstrapDispatch.h"
    #include "core/assert.hpp"
    #include "core/window.hpp"
    #include "gfx/backend/vulkan/conversions.hpp"

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
    #define VK_CHECK(result, message)                                             \
        {                                                                         \
            MAG_ASSERT(result == VK_SUCCESS, "Vk check failed: " + str(message)); \
        }

    namespace gfx
    {
        class VulkanSemaphore : public ISemaphore
        {
            public:
                VulkanSemaphore(const vkb::DispatchTable& disp, const ISemaphoreDesc& desc) : disp(disp)
                {
                    (void)desc;

                    VkSemaphoreCreateInfo semaphore_info = {};
                    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                    VK_CHECK(disp.createSemaphore(&semaphore_info, nullptr, &semaphore), "Failed to create semaphore");
                }

                ~VulkanSemaphore() { disp.destroySemaphore(semaphore, nullptr); }

                const VkSemaphore& get_semaphore() const { return semaphore; }

            private:
                const vkb::DispatchTable& disp;
                VkSemaphore semaphore;
        };

        class VulkanFence : public IFence
        {
            public:
                VulkanFence(const vkb::DispatchTable& disp, const IFenceDesc& desc) : disp(disp)
                {
                    VkFenceCreateInfo fence_info = {};
                    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

                    if (desc.signaled)
                    {
                        fence_info.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
                    }

                    VK_CHECK(disp.createFence(&fence_info, nullptr, &fence), "Failed to create fence");
                }

                ~VulkanFence() { disp.destroyFence(fence, nullptr); }

                virtual void wait(const u64 timeout) override { disp.waitForFences(1, &fence, VK_TRUE, timeout); }

                virtual void reset() override { disp.resetFences(1, &fence); }

                const VkFence& get_fence() const { return fence; }

            private:
                const vkb::DispatchTable& disp;
                VkFence fence;
        };

        class VulkanBuffer : public IBuffer
        {
            public:
                VulkanBuffer(const IBufferDesc& desc, const VmaAllocator& allocator)
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

                    VK_CHECK(vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &buffer,
                                             &allocation, nullptr),
                             "Failed to create buffer");

                    // Use persistent mapping
                    this->map();
                }

                ~VulkanBuffer()
                {
                    this->unmap();
                    vmaDestroyBuffer(allocator, buffer, allocation);
                }

                virtual void* map() override
                {
                    VK_CHECK(vmaMapMemory(allocator, allocation, &mapped_region), "Failed to map buffer memory");
                    return mapped_region;
                }

                virtual void unmap() override { vmaUnmapMemory(allocator, allocation); }

                virtual void set_data(const void* data, const u64 size, const u64 offset = 0) override
                {
                    MAG_ASSERT(offset + size <= size, "Size limit exceeded");
                    memcpy(static_cast<c8*>(mapped_region) + offset, data, size);
                }

                virtual u64 get_size() const override { return size; }

                virtual BufferUsage get_usage() const override { return usage; }

                const VkBuffer& get_buffer() const { return buffer; }

            private:
                const VmaAllocator& allocator;
                u64 size = 0;
                BufferUsage usage;
                VkBuffer buffer;
                VmaAllocation allocation = nullptr;
                void* mapped_region = nullptr;
        };

        class VulkanSampler : public ISampler
        {
            public:
                VulkanSampler(const ISamplerDesc& desc, const vkb::DispatchTable& disp) : disp(disp)
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
                    sampler_info.anisotropyEnable = desc.anisotropy_enable;
                    sampler_info.maxAnisotropy = desc.max_anisotropy;

                    disp.createSampler(&sampler_info, nullptr, &sampler);
                }

                ~VulkanSampler() { disp.destroySampler(sampler, nullptr); }

                const VkSampler& get_sampler() const { return sampler; }

            private:
                const vkb::DispatchTable& disp;
                VkSampler sampler;
        };

        class VulkanTexture : public ITexture
        {
            public:
                VulkanTexture(const vkb::DispatchTable& disp, const VmaAllocator& allocator, IDevice* device,
                              const ITextureDesc& desc)
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
                    VkImageCreateInfo image_create_info = {};
                    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                    image_create_info.imageType = mag_to_vk(desc.type);
                    image_create_info.format = mag_to_vk(desc.format);
                    image_create_info.extent = mag_to_vk(desc.extent);
                    image_create_info.mipLevels = desc.mip_levels;
                    image_create_info.arrayLayers = desc.array_layers;
                    image_create_info.samples = mag_to_vk(desc.sample_count);
                    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
                    image_create_info.usage = mag_to_vk(desc.usage);

                    VmaAllocationCreateInfo vma_alloc_info = {};
                    vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                    vma_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

                    VK_CHECK(vmaCreateImage(allocator, &image_create_info, &vma_alloc_info, &image, &allocation, 0),
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

                    VK_CHECK(disp.createImageView(&view_create_info, nullptr, &image_view),
                             "Failed to create image view");
                }

                // Special case for swapchain images
                VulkanTexture(const vkb::DispatchTable& disp, const VmaAllocator& allocator, const VkImage image,
                              const VkImageView image_view)
                    : disp(disp),
                      allocator(allocator),
                      image(image),
                      image_view(image_view),
                      usage(TextureUsage::TransferDst)
                {
                }

                ~VulkanTexture()
                {
                    disp.destroyImageView(image_view, nullptr);
                    if (allocation != nullptr)
                    {
                        vmaDestroyImage(allocator, image, allocation);
                    }
                }

                virtual void set_data(const void* data, const u64 size) override
                {
                    IBufferDesc staging_buffer_desc = {};
                    staging_buffer_desc.buffer_usage = BufferUsage::TransferSrc;
                    staging_buffer_desc.memory_usage = MemoryUsage::Auto;
                    staging_buffer_desc.size_bytes = size;

                    unique<IBuffer> staging_buffer = device->create_buffer(staging_buffer_desc);
                    staging_buffer->set_data(data, size);

                    // @TODO: use KTX to generate mip maps: https://www.khronos.org/ktx/

                    device->submit_commands_immediate(
                        [&](ICommandBuffer& cmd)
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

                virtual const math::uvec3& get_extent() const override { return extent; }

                virtual Format get_format() const override { return format; }

                virtual TextureLayout get_layout() const override { return layout; }

                virtual TextureType get_type() const override { return type; }

                virtual TextureViewType get_view_type() const override { return view_type; }

                virtual TextureAspect get_aspect() const override { return aspect; }

                virtual TextureUsage get_usage() const override { return usage; }

                virtual SampleCount get_sample_count() const override { return sample_count; }

                virtual u32 get_mip_levels() const override { return mip_levels; }

                virtual u32 get_array_layers() const override { return array_layers; }

                const VkImage& get_image() const { return image; }

                const VkImageView& get_image_view() const { return image_view; }

                void set_new_layout(const VkImageLayout new_image_layout) { layout = vk_to_mag(new_image_layout); }

            private:
                const vkb::DispatchTable& disp;
                const VmaAllocator& allocator;
                IDevice* device = nullptr;
                VkImage image = {};
                VkImageView image_view = {};
                VmaAllocation allocation = nullptr;
                math::uvec3 extent = {1.0f, 1.0f, 1.0f};
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
                VulkanSwapchain(const vkb::DispatchTable& disp, const vkb::Device& device, const ISwapchainDesc& desc)
                    : disp(disp)
                {
                    vkb::SwapchainBuilder swapchain_builder{device};
                    const auto swap_ret = swapchain_builder.set_old_swapchain(swapchain)
                                              .set_desired_present_mode(mag_to_vk(desc.desired_present_mode))
                                              .add_fallback_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
                                              .add_fallback_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
                                              .add_fallback_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                              .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                              .build();

                    MAG_ASSERT(swap_ret, swap_ret.error().message() + " " + std::to_string(swap_ret.vk_result()));

                    vkb::destroy_swapchain(swapchain);

                    swapchain = swap_ret.value();

                    const std::vector<VkImage>& swapchain_images = swapchain.get_images().value();
                    const std::vector<VkImageView>& swapchain_image_views = swapchain.get_image_views().value();
                    for (u32 i = 0; i < swapchain.image_count; i++)
                    {
                        VulkanTexture* texture =
                            new VulkanTexture(disp, nullptr, swapchain_images[i], swapchain_image_views[i]);
                        swapchain_textures.emplace_back(texture);
                    }
                }

                ~VulkanSwapchain()
                {
                    vkb::destroy_swapchain(swapchain);
                    swapchain_textures.clear();
                }

                virtual u32 get_current_image_index() const override { return current_image_index; }

                virtual u32 get_image_count() const override { return swapchain.image_count; }

                virtual math::uvec2 get_extent() const override { return vk_to_mag(swapchain.extent); }

                virtual Format get_format() const override { return vk_to_mag(swapchain.image_format); }

                virtual const ITexture* get_texture(const u32 index) const override
                {
                    return swapchain_textures[index].get();
                }

                virtual b8 acquire_next_image(const ISemaphore* signal_semaphore,
                                              const IFence* fence = nullptr) override
                {
                    const VkSemaphore vk_sem = static_cast<const VulkanSemaphore*>(signal_semaphore)->get_semaphore();
                    VkFence vk_fen = nullptr;

                    if (fence != nullptr)
                    {
                        vk_fen = static_cast<const VulkanFence*>(fence)->get_fence();
                    }

                    const VkResult result =
                        disp.acquireNextImageKHR(swapchain, Timeout, vk_sem, vk_fen, &current_image_index);

                    if (result == VK_ERROR_OUT_OF_DATE_KHR)
                    {
                        resize({});
                    }

                    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                    {
                        MAG_ASSERT(false, "Failed to acquire swapchain image");
                    }

                    return true;
                }

                virtual b8 resize(const math::uvec2& extent) override { MAG_ASSERT(false, "@TODO"); }

                const VkSwapchainKHR& get_swapchain() const { return swapchain.swapchain; }

            private:
                const vkb::DispatchTable& disp;
                vkb::Swapchain swapchain;
                u32 current_image_index = 0;
                std::vector<unique<VulkanTexture>> swapchain_textures;
        };

        class VulkanDescriptorPool : public IDescriptorPool
        {
            public:
                VulkanDescriptorPool(const IDescriptorPoolDesc& desc, const vkb::DispatchTable& disp) : disp(disp)
                {
                    std::vector<VkDescriptorPoolSize> pool_sizes;
                    for (const IDescriptorPoolSizeDesc& size_desc : desc.size_descs)
                    {
                        VkDescriptorPoolSize pool_size = {};
                        pool_size.type = mag_to_vk(size_desc.type);
                        pool_size.descriptorCount = size_desc.size;

                        pool_sizes.push_back(pool_size);
                    }

                    VkDescriptorPoolCreateInfo pool_info = {};
                    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    pool_info.poolSizeCount = pool_sizes.size();
                    pool_info.pPoolSizes = pool_sizes.data();
                    pool_info.maxSets = desc.max_sets;
                    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT |
                                      VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

                    VK_CHECK(disp.createDescriptorPool(&pool_info, nullptr, &descriptor_pool),
                             "Failed to create descriptor pool");
                }

                ~VulkanDescriptorPool() { disp.destroyDescriptorPool(descriptor_pool, nullptr); }

                const VkDescriptorPool& get_pool() const { return descriptor_pool; }

            private:
                const vkb::DispatchTable& disp;
                VkDescriptorPool descriptor_pool;
        };

        class VulkanDescriptorSetLayout : public IDescriptorSetLayout
        {
            public:
                VulkanDescriptorSetLayout(const IDescriptorSetLayoutDesc& desc, const vkb::DispatchTable& disp)
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

                        if (binding_desc.binding > 0)
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

                    VK_CHECK(disp.createDescriptorSetLayout(&layout_info, nullptr, &descriptor_layout),
                             "Failed to create descriptor set layout");
                }

                ~VulkanDescriptorSetLayout() { disp.destroyDescriptorSetLayout(descriptor_layout, nullptr); }

                const VkDescriptorSetLayout& get_layout() const { return descriptor_layout; }

            private:
                const vkb::DispatchTable& disp;
                VkDescriptorSetLayout descriptor_layout;
        };

        class VulkanDescriptorSet : public IDescriptorSet
        {
            public:
                VulkanDescriptorSet(const IDescriptorSetDesc& desc, const vkb::DispatchTable& disp) : disp(disp)
                {
                    parent_pool = ((VulkanDescriptorPool*)desc.descriptor_pool)->get_pool();

                    VkDescriptorSetLayout descriptor_layout =
                        ((VulkanDescriptorSetLayout*)desc.descriptor_layout)->get_layout();

                    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variable_count_info = {};
                    variable_count_info.sType =
                        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    variable_count_info.descriptorSetCount = 1;
                    variable_count_info.pDescriptorCounts = &desc.max_descriptor_count;

                    VkDescriptorSetAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    alloc_info.descriptorPool = parent_pool;
                    alloc_info.descriptorSetCount = 1;
                    alloc_info.pSetLayouts = &descriptor_layout;
                    alloc_info.pNext = &variable_count_info;

                    VK_CHECK(disp.allocateDescriptorSets(&alloc_info, &descriptor_set),
                             "Failed to allocate descriptor sets");
                }

                ~VulkanDescriptorSet() { disp.freeDescriptorSets(parent_pool, 1, &descriptor_set); }

                virtual void update(const IBuffer* const buffer, const u32 binding, const u32 array_element,
                                    const DescriptorType descriptor_type, const u64 offset) override
                {
                    std::vector<VkWriteDescriptorSet> descriptor_writes;

                    VkDescriptorBufferInfo buffer_info = {};
                    buffer_info.buffer = ((VulkanBuffer*)buffer)->get_buffer();
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

                    disp.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
                }

                virtual void update(const ITexture* const texture, const ISampler* const sampler, const u32 binding,
                                    const u32 array_element, const DescriptorType descriptor_type) override
                {
                    std::vector<VkWriteDescriptorSet> descriptor_writes;

                    VkDescriptorImageInfo image_info = {};
                    image_info.imageLayout = mag_to_vk(texture->get_layout());
                    image_info.imageView = ((VulkanTexture*)texture)->get_image_view();
                    image_info.sampler = ((VulkanSampler*)sampler)->get_sampler();

                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptor_set;
                    write.dstBinding = binding;
                    write.dstArrayElement = array_element;
                    write.descriptorType = mag_to_vk(descriptor_type);
                    write.descriptorCount = 1;
                    write.pImageInfo = &image_info;

                    descriptor_writes.push_back(write);

                    disp.updateDescriptorSets(descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
                }

                const VkDescriptorSet& get_descriptor_set() const { return descriptor_set; }

            private:
                const vkb::DispatchTable& disp;
                VkDescriptorSet descriptor_set;
                VkDescriptorPool parent_pool;
        };

        class VulkanGraphicsPipeline : public IGraphicsPipeline
        {
            public:
                VulkanGraphicsPipeline(const vkb::DispatchTable& disp, const IGraphicsPipelineDesc& desc) : disp(disp)
                {
                    const u32 shader_module_count = desc.shader_modules.size();

                    VkPipelineShaderStageCreateInfo shader_stages[shader_module_count];
                    VkShaderModule shader_modules[shader_module_count];

                    for (u32 i = 0; i < shader_module_count; i++)
                    {
                        const IShaderModuleDesc& shader_module_desc = desc.shader_modules[i];

                        VkShaderModuleCreateInfo shader_module_info = {};
                        shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        shader_module_info.codeSize = shader_module_desc.code.size();
                        shader_module_info.pCode = reinterpret_cast<const u32*>(shader_module_desc.code.data());

                        shader_modules[i] = {};

                        VK_CHECK(disp.createShaderModule(&shader_module_info, nullptr, &shader_modules[i]),
                                 "Failed to create shader module");

                        shader_stages[i] = {};
                        shader_stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                        shader_stages[i].stage = mag_to_vk_bits(shader_module_desc.stage);
                        shader_stages[i].module = shader_modules[i];
                        shader_stages[i].pName = "main";
                    }

                    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
                    for (const IDescriptorSetLayout* descriptor_layout : desc.descriptor_layouts)
                    {
                        const VkDescriptorSetLayout descriptor_set_layout =
                            ((VulkanDescriptorSetLayout*)descriptor_layout)->get_layout();

                        descriptor_set_layouts.push_back(descriptor_set_layout);
                    }

                    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
                    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                    vertex_input_info.vertexBindingDescriptionCount = 0;
                    vertex_input_info.vertexAttributeDescriptionCount = 0;

                    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
                    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                    input_assembly.topology = mag_to_vk(desc.primitive_topology);
                    input_assembly.primitiveRestartEnable = VK_FALSE;

                    VkViewport viewport = {};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = static_cast<f32>(desc.extent.x);
                    viewport.height = static_cast<f32>(desc.extent.y);
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;

                    VkRect2D scissor = {};
                    scissor.offset = {0, 0};
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
                    rasterizer.lineWidth = 1.0f;
                    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
                    rasterizer.depthBiasEnable = VK_FALSE;

                    VkPipelineMultisampleStateCreateInfo multisampling = {};
                    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                    multisampling.sampleShadingEnable = VK_FALSE;
                    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

                    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
                    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                    color_blend_attachment.blendEnable = VK_FALSE;

                    VkPipelineColorBlendStateCreateInfo color_blending = {};
                    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                    color_blending.logicOpEnable = VK_FALSE;
                    color_blending.logicOp = VK_LOGIC_OP_COPY;
                    color_blending.attachmentCount = 1;
                    color_blending.pAttachments = &color_blend_attachment;
                    color_blending.blendConstants[0] = 0.0f;
                    color_blending.blendConstants[1] = 0.0f;
                    color_blending.blendConstants[2] = 0.0f;
                    color_blending.blendConstants[3] = 0.0f;

                    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
                    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                    pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
                    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
                    pipeline_layout_info.pushConstantRangeCount = 0;

                    if (disp.createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create pipeline layout");
                    }

                    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

                    VkPipelineDynamicStateCreateInfo dynamic_info = {};
                    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                    dynamic_info.dynamicStateCount = static_cast<u32>(dynamic_states.size());
                    dynamic_info.pDynamicStates = dynamic_states.data();

                    VkFormat swapchain_format = mag_to_vk(desc.format);

                    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
                    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
                    pipeline_rendering_create_info.colorAttachmentCount = 1;
                    pipeline_rendering_create_info.pColorAttachmentFormats = &swapchain_format;

                    VkGraphicsPipelineCreateInfo pipeline_info = {};
                    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                    pipeline_info.stageCount = shader_module_count;
                    pipeline_info.pStages = shader_stages;
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

                    if (disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) !=
                        VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create pipeline");
                    }

                    for (u32 i = 0; i < shader_module_count; i++)
                    {
                        disp.destroyShaderModule(shader_modules[i], nullptr);
                    }
                }

                ~VulkanGraphicsPipeline()
                {
                    disp.destroyPipeline(pipeline, nullptr);
                    disp.destroyPipelineLayout(pipeline_layout, nullptr);
                }

                const VkPipeline& get_pipeline() const { return pipeline; }

                const VkPipelineLayout& get_pipeline_layout() const { return pipeline_layout; }

            private:
                const vkb::DispatchTable& disp;
                VkPipelineLayout pipeline_layout;
                VkPipeline pipeline;
        };

        class VulkanRenderingAttachment : public IRenderingAttachment
        {
            public:
                VulkanRenderingAttachment(const IRenderingAttachmentDesc& desc)
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
                    rendering_attachment_info.imageView = ((VulkanTexture*)desc.texture)->get_image_view();
                    rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    rendering_attachment_info.clearValue = clear_value;
                }

                ~VulkanRenderingAttachment() {}

                virtual math::vec4 get_clear_color() const override
                {
                    return vk_to_mag(rendering_attachment_info.clearValue.color);
                }

                virtual f32 get_clear_depth() const override
                {
                    return rendering_attachment_info.clearValue.depthStencil.depth;
                }

                virtual u32 get_clear_stencil() const override
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
                VulkanRenderPass(const IRenderPassDesc& desc)
                {
                    VkRect2D render_area = {};
                    render_area.extent = mag_to_vk(desc.extent);
                    render_area.offset = mag_to_vk(desc.offset);

                    for (const IRenderingAttachment* color_attachment : desc.color_attachments)
                    {
                        color_attachments.push_back(
                            ((VulkanRenderingAttachment*)color_attachment)->get_attachment_info());
                    }

                    render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
                    render_info.renderArea = render_area;
                    render_info.layerCount = 1;
                    render_info.colorAttachmentCount = color_attachments.size();
                    render_info.pColorAttachments = color_attachments.data();
                }

                ~VulkanRenderPass() {}

                virtual math::ivec2 get_offset() const { return vk_to_mag(render_info.renderArea.offset); }

                virtual math::uvec2 get_extent() const { return vk_to_mag(render_info.renderArea.extent); }

                const VkRenderingInfoKHR& get_rendering_info() const { return render_info; }

            private:
                VkRenderingInfoKHR render_info = {};
                std::vector<VkRenderingAttachmentInfo> color_attachments;
        };

        class VulkanCommandPool : public ICommandPool
        {
            public:
                VulkanCommandPool(const vkb::DispatchTable& disp, const vkb::Device& device,
                                  const ICommandPoolDesc& desc)
                    : disp(disp)
                {
                    VkCommandPoolCreateInfo pool_info = {};
                    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    pool_info.queueFamilyIndex = device.get_queue_index(mag_to_vk(desc.queue_type)).value();

                    VK_CHECK(disp.createCommandPool(&pool_info, nullptr, &pool), "Failed to create command pool");
                }

                ~VulkanCommandPool() { disp.destroyCommandPool(pool, nullptr); }

                virtual void reset() override { disp.resetCommandPool(pool, 0); }

                const VkCommandPool& get_pool() const { return pool; }

            private:
                const vkb::DispatchTable& disp;
                VkCommandPool pool;
        };

        class VulkanCommandBuffer : public ICommandBuffer
        {
            public:
                VulkanCommandBuffer(const vkb::DispatchTable& disp, const ICommandBufferDesc& desc) : disp(disp)
                {
                    command_pool = (VulkanCommandPool*)desc.command_pool;

                    VkCommandBufferAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    alloc_info.commandPool = command_pool->get_pool();
                    alloc_info.level = mag_to_vk(desc.command_buffer_level);
                    alloc_info.commandBufferCount = 1;

                    VK_CHECK(disp.allocateCommandBuffers(&alloc_info, &command_buffer),
                             "Failed to allocate command buffer");
                }

                ~VulkanCommandBuffer() { disp.freeCommandBuffers(command_pool->get_pool(), 1, &command_buffer); }

                virtual void begin_recording() override
                {
                    VkCommandBufferBeginInfo begin_info = {};
                    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                    VK_CHECK(disp.beginCommandBuffer(command_buffer, &begin_info),
                             "Failed to begin command buffer recording");
                }

                virtual void end_recording() override
                {
                    VK_CHECK(disp.endCommandBuffer(command_buffer), "Failed to record command buffer");
                }

                virtual void reset() override { disp.resetCommandBuffer(command_buffer, 0); }

                virtual void set_viewport(const math::vec2& extent, const math::vec2& offset, const f32 min_depth,
                                          const f32 max_depth) override
                {
                    VkViewport viewport = {};
                    viewport.width = extent.x;
                    viewport.height = extent.y;
                    viewport.x = offset.x;
                    viewport.y = offset.y;
                    viewport.minDepth = min_depth;
                    viewport.maxDepth = max_depth;

                    disp.cmdSetViewport(command_buffer, 0, 1, &viewport);
                }

                virtual void set_scissor(const math::uvec2& extent, const math::ivec2& offset = {0.0f, 0.0f}) override
                {
                    VkRect2D scissor = {};
                    scissor.extent = mag_to_vk(extent);
                    scissor.offset = mag_to_vk(offset);

                    disp.cmdSetScissor(command_buffer, 0, 1, &scissor);
                }

                virtual void begin_rendering(const IRenderPass* render_pass) override
                {
                    disp.cmdBeginRendering(command_buffer, &((VulkanRenderPass*)render_pass)->get_rendering_info());
                }

                virtual void end_rendering() override { disp.cmdEndRenderingKHR(command_buffer); }

                virtual void bind_pipeline(const IGraphicsPipeline* pipeline) override
                {
                    disp.cmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                         ((VulkanGraphicsPipeline*)pipeline)->get_pipeline());
                }

                virtual void bind_descriptor(const IGraphicsPipeline* pipeline,
                                             const IDescriptorSet* descriptor) override
                {
                    disp.cmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               ((VulkanGraphicsPipeline*)pipeline)->get_pipeline_layout(), 0, 1,
                                               &((VulkanDescriptorSet*)descriptor)->get_descriptor_set(), 0, nullptr);
                }

                virtual void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                                  const u32 first_instance) override
                {
                    disp.cmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
                }

                virtual void pipeline_barrier(const ITexture* texture, const TextureLayout new_layout,
                                              const AccessMask src_access_mask, const AccessMask dst_access_mask,
                                              const PipelineStage src_stage_mask,
                                              const PipelineStage dst_stage_mask) override
                {
                    VkImageMemoryBarrier image_memory_barrier = {};
                    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_memory_barrier.srcAccessMask = mag_to_vk(src_access_mask);
                    image_memory_barrier.dstAccessMask = mag_to_vk(dst_access_mask);
                    image_memory_barrier.oldLayout = mag_to_vk(texture->get_layout());
                    image_memory_barrier.newLayout = mag_to_vk(new_layout);
                    image_memory_barrier.image = ((VulkanTexture*)texture)->get_image();
                    image_memory_barrier.subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    };

                    disp.cmdPipelineBarrier(command_buffer, mag_to_vk(src_stage_mask), mag_to_vk(dst_stage_mask),
                                            VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                                            &image_memory_barrier);

                    ((VulkanTexture*)texture)->set_new_layout(mag_to_vk(new_layout));
                }

                virtual void copy_texture(const ITexture* src_texture, const ITexture* dst_texture) override
                {
                    const VulkanTexture* vk_src = static_cast<const VulkanTexture*>(src_texture);
                    const VulkanTexture* vk_dst = static_cast<const VulkanTexture*>(dst_texture);

                    VkImageSubresourceLayers src_subresource = {};
                    src_subresource.layerCount = src_texture->get_array_layers();
                    src_subresource.aspectMask = mag_to_vk(vk_src->get_aspect());

                    VkImageSubresourceLayers dst_subresource = {};
                    dst_subresource.layerCount = dst_texture->get_array_layers();
                    dst_subresource.aspectMask = mag_to_vk(vk_dst->get_aspect());

                    auto src_extent = vk_src->get_extent();

                    VkOffset3D src_offset = {};
                    VkOffset3D dst_offset = {};

                    VkImageCopy image_copy = {};
                    image_copy.extent = mag_to_vk(src_extent);
                    image_copy.srcOffset = src_offset;
                    image_copy.srcSubresource = src_subresource;
                    image_copy.dstOffset = dst_offset;
                    image_copy.dstSubresource = dst_subresource;

                    disp.cmdCopyImage(command_buffer, vk_src->get_image(), mag_to_vk(vk_src->get_layout()),
                                      vk_dst->get_image(), mag_to_vk(vk_dst->get_layout()), 1, &image_copy);
                }

                virtual void copy_buffer_to_texture(const IBuffer* buffer, const ITexture* texture) override
                {
                    VulkanBuffer* vk_buffer = (VulkanBuffer*)buffer;
                    VulkanTexture* vk_texture = (VulkanTexture*)texture;

                    VkBufferImageCopy buffer_image_copy = {};
                    buffer_image_copy.bufferImageHeight = texture->get_extent().y;
                    buffer_image_copy.bufferRowLength = texture->get_extent().x;
                    buffer_image_copy.imageExtent = mag_to_vk(texture->get_extent());
                    buffer_image_copy.imageSubresource.aspectMask = mag_to_vk(texture->get_aspect());
                    buffer_image_copy.imageSubresource.baseArrayLayer = 0;
                    buffer_image_copy.imageSubresource.layerCount = texture->get_array_layers();
                    buffer_image_copy.imageSubresource.mipLevel = 0;

                    disp.cmdCopyBufferToImage(command_buffer, vk_buffer->get_buffer(), vk_texture->get_image(),
                                              mag_to_vk(vk_texture->get_layout()), 1, &buffer_image_copy);
                }

                const VkCommandBuffer& get_command_buffer() const { return command_buffer; }

            private:
                const vkb::DispatchTable& disp;
                const VulkanCommandPool* command_pool;
                VkCommandBuffer command_buffer;
        };

        class VulkanQueue : public IQueue
        {
            public:
                VulkanQueue(const vkb::DispatchTable& disp, const vkb::Device& device, const IQueueDesc& desc)
                    : disp(disp)
                {
                    const auto queue_ret = device.get_queue(mag_to_vk(desc.queue_type));

                    MAG_ASSERT(queue_ret, queue_ret.error().message());

                    queue = queue_ret.value();
                }

                ~VulkanQueue() {}

                virtual void submit(const ISemaphore* wait_semaphore, const ISemaphore* signal_semaphore, IFence* fence,
                                    const ICommandBuffer* command_buffer) override
                {
                    VkSubmitInfo submit_info = {};
                    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                    submit_info.pWaitDstStageMask = wait_stages;

                    if (wait_semaphore)
                    {
                        submit_info.waitSemaphoreCount = 1;
                        submit_info.pWaitSemaphores = &((VulkanSemaphore*)wait_semaphore)->get_semaphore();
                    }

                    if (signal_semaphore)
                    {
                        submit_info.signalSemaphoreCount = 1;
                        submit_info.pSignalSemaphores = &((VulkanSemaphore*)signal_semaphore)->get_semaphore();
                    }

                    if (command_buffer)
                    {
                        submit_info.commandBufferCount = 1;
                        submit_info.pCommandBuffers = &((VulkanCommandBuffer*)command_buffer)->get_command_buffer();
                    }

                    fence->reset();

                    VK_CHECK(disp.queueSubmit(queue, 1, &submit_info, ((VulkanFence*)fence)->get_fence()),
                             "Failed to submit draw command buffer");
                }

                virtual i32 present(const ISwapchain* swapchain, const ISemaphore* wait_semaphore) override
                {
                    const u32 image_index = ((VulkanSwapchain*)swapchain)->get_current_image_index();

                    VkPresentInfoKHR present_info = {};
                    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                    present_info.waitSemaphoreCount = 1;
                    present_info.pWaitSemaphores = &((VulkanSemaphore*)wait_semaphore)->get_semaphore();
                    present_info.swapchainCount = 1;
                    present_info.pSwapchains = &((VulkanSwapchain*)swapchain)->get_swapchain();
                    present_info.pImageIndices = &image_index;

                    const VkResult result = disp.queuePresentKHR(queue, &present_info);

                    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
                    {
                        MAG_ASSERT(false, "@TODO: resize swapchain");
                    }
                    else if (result != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to present swapchain image");
                    }

                    return result;
                }

            private:
                const vkb::DispatchTable& disp;
                VkQueue queue;
        };

        class VulkanDevice : public IDevice
        {
            public:
                VulkanDevice()
                {
                    // Device
                    // -------------------------------------------------------------------------------------------------
                    vkb::InstanceBuilder instance_builder;
                    const auto instance_ret = instance_builder

    #if MAG_CONFIG_DEBUG
                                                  .use_default_debug_messenger()
                                                  .request_validation_layers()
    #endif
                                                  .require_api_version(1, 3, 0)
                                                  .build();

                    MAG_ASSERT(instance_ret, instance_ret.error().message());

                    instance = instance_ret.value();
                    inst_disp = instance.make_table();

                    window::create_surface(&instance.instance, &surface);

                    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {};
                    dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
                    dynamic_rendering_feature.dynamicRendering = true;

                    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {};
                    descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
                    descriptor_indexing_features.descriptorBindingPartiallyBound = true;
                    descriptor_indexing_features.descriptorBindingVariableDescriptorCount = true;
                    descriptor_indexing_features.descriptorBindingUniformBufferUpdateAfterBind = true;
                    descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind = true;
                    descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind = true;
                    descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = true;
                    descriptor_indexing_features.shaderStorageBufferArrayNonUniformIndexing = true;
                    descriptor_indexing_features.shaderUniformBufferArrayNonUniformIndexing = true;

                    vkb::PhysicalDeviceSelector phys_device_selector(instance);
                    const auto phys_device_ret = phys_device_selector.set_minimum_version(1, 3)
                                                     .set_surface(surface)
                                                     .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                                                     .add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
                                                     .add_required_extension_features(descriptor_indexing_features)
                                                     .select();

                    MAG_ASSERT(phys_device_ret, phys_device_ret.error().message());

                    vkb::PhysicalDevice physical_device = phys_device_ret.value();
                    vkb::DeviceBuilder device_builder{physical_device};
                    const auto device_ret = device_builder.add_pNext(&dynamic_rendering_feature).build();

                    MAG_ASSERT(device_ret, device_ret.error().message());

                    device = device_ret.value();

                    disp = device.make_table();

                    VmaAllocatorCreateInfo allocator_create_info = {};
                    allocator_create_info.physicalDevice = physical_device.physical_device;
                    allocator_create_info.device = device.device;
                    allocator_create_info.instance = instance.instance;
                    allocator_create_info.vulkanApiVersion = instance.api_version;
                    // allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

                    VK_CHECK(vmaCreateAllocator(&allocator_create_info, &allocator),
                             "Failed to create memory allocator");

                    // Immediate submission resources

                    ICommandPoolDesc command_pool_desc = {};
                    command_pool_desc.queue_type = QueueType::Graphics;
                    immediate_command_pool = this->create_command_pool(command_pool_desc);

                    ICommandBufferDesc command_buffer_desc = {};
                    command_buffer_desc.command_buffer_level = CommandBufferLevel::Primary;
                    command_buffer_desc.command_pool = immediate_command_pool.get();
                    immediate_command_buffer = this->create_command_buffer(command_buffer_desc);

                    IQueueDesc queue_desc = {};
                    queue_desc.queue_type = QueueType::Graphics;
                    immediate_queue = this->create_queue(queue_desc);

                    IFenceDesc fence_desc = {};
                    immediate_fence = this->create_fence(fence_desc);
                }

                ~VulkanDevice()
                {
                    disp.deviceWaitIdle();

                    immediate_fence.reset();
                    immediate_queue.reset();
                    immediate_command_buffer.reset();
                    immediate_command_pool.reset();

                    vmaDestroyAllocator(allocator);

                    vkb::destroy_device(device);
                    vkb::destroy_surface(instance, surface);
                    vkb::destroy_instance(instance);
                }

                virtual void wait_idle() override { disp.deviceWaitIdle(); }

                virtual void submit_commands_immediate(std::function<void(ICommandBuffer& cmd)>&& function) override
                {
                    const unique<ICommandBuffer>& cmd = immediate_command_buffer;

                    cmd->begin_recording();
                    function(*cmd);  // execute the function
                    cmd->end_recording();

                    immediate_queue->submit(nullptr, nullptr, immediate_fence.get(), immediate_command_buffer.get());
                    immediate_fence->wait();

                    immediate_fence->reset();
                    immediate_command_pool->reset();
                }

                virtual unique<ISemaphore> create_semaphore(const ISemaphoreDesc& desc) override
                {
                    return create_unique<VulkanSemaphore>(disp, desc);
                }

                virtual unique<IFence> create_fence(const IFenceDesc& desc) override
                {
                    return create_unique<VulkanFence>(disp, desc);
                }

                virtual unique<ISwapchain> create_swapchain(const ISwapchainDesc& desc) override
                {
                    return create_unique<VulkanSwapchain>(disp, device, desc);
                }

                virtual unique<IQueue> create_queue(const IQueueDesc& desc) override
                {
                    return create_unique<VulkanQueue>(disp, device, desc);
                }

                virtual unique<IGraphicsPipeline> create_graphics_pipeline(const IGraphicsPipelineDesc& desc) override
                {
                    return create_unique<VulkanGraphicsPipeline>(disp, desc);
                }

                virtual unique<ICommandPool> create_command_pool(const ICommandPoolDesc& desc) override
                {
                    return create_unique<VulkanCommandPool>(disp, device, desc);
                }

                virtual unique<ICommandBuffer> create_command_buffer(const ICommandBufferDesc& desc) override
                {
                    return create_unique<VulkanCommandBuffer>(disp, desc);
                }

                virtual unique<IRenderingAttachment> create_render_attachment(
                    const IRenderingAttachmentDesc& desc) override
                {
                    return create_unique<VulkanRenderingAttachment>(desc);
                }

                virtual unique<IRenderPass> create_render_pass(const IRenderPassDesc& desc) override
                {
                    return create_unique<VulkanRenderPass>(desc);
                }

                virtual unique<ITexture> create_texture(const ITextureDesc& desc) override
                {
                    return create_unique<VulkanTexture>(disp, allocator, this, desc);
                }

                virtual unique<IBuffer> create_buffer(const IBufferDesc& desc) override
                {
                    return create_unique<VulkanBuffer>(desc, allocator);
                }

                virtual unique<IDescriptorPool> create_descriptor_pool(const IDescriptorPoolDesc& desc) override
                {
                    return create_unique<VulkanDescriptorPool>(desc, disp);
                }

                virtual unique<IDescriptorSetLayout> create_descriptor_set_layout(
                    const IDescriptorSetLayoutDesc& desc) override
                {
                    return create_unique<VulkanDescriptorSetLayout>(desc, disp);
                }

                virtual unique<IDescriptorSet> create_descriptor_set(const IDescriptorSetDesc& desc) override
                {
                    return create_unique<VulkanDescriptorSet>(desc, disp);
                }

                virtual unique<ISampler> create_sampler(const ISamplerDesc& desc) override
                {
                    return create_unique<VulkanSampler>(desc, disp);
                }

            private:
                vkb::Instance instance;
                vkb::Device device;
                vkb::InstanceDispatchTable inst_disp;
                vkb::DispatchTable disp;
                VkSurfaceKHR surface;
                VmaAllocator allocator;

                unique<ICommandBuffer> immediate_command_buffer;
                unique<ICommandPool> immediate_command_pool;
                unique<IQueue> immediate_queue;
                unique<IFence> immediate_fence;
        };

        unique<IDevice> create_device() { return create_unique<VulkanDevice>(); }
    };  // namespace gfx
};      // namespace mag

#endif
