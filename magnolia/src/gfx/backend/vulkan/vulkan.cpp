#include "gfx/gfx.hpp"

#define MAG_CONFIG_GFX_VULKAN 1

#if MAG_CONFIG_GFX_VULKAN

    #include <vulkan/vulkan.h>

    #include <string>

    #include "VkBootstrap.h"
    #include "VkBootstrapDispatch.h"
    #include "core/assert.hpp"
    #include "core/buffer.hpp"
    #include "core/window.hpp"
    #include "gfx/backend/vulkan/conversions.hpp"
    #include "platform/file_system.hpp"

namespace mag
{
    // @TODO: temporary
    #define EXAMPLE_BUILD_DIRECTORY "magnolia/assets/shaders"
    #define MAX_FRAMES_IN_FLIGHT 3

    #define VK_CHECK(result, message)                                             \
        {                                                                         \
            MAG_ASSERT(result == VK_SUCCESS, "Vk check failed: " + str(message)); \
        }

    namespace gfx
    {
        class VulkanSemaphore : public ISemaphore
        {
            public:
                VulkanSemaphore(const vkb::DispatchTable& disp) : disp(disp)
                {
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

        class VulkanTexture : public ITexture
        {
            public:
                VulkanTexture(const vkb::DispatchTable& disp, const ITextureDesc& desc) : disp(disp)
                {
                    MAG_ASSERT(false, "@TODO");
                }

                // Special case for swapchain images (simply copy image and view)
                VulkanTexture(const vkb::DispatchTable& disp, const VkImage image, const VkImageView image_view)
                    : disp(disp), image(image), image_view(image_view)
                {
                }

                ~VulkanTexture() { disp.destroyImageView(image_view, nullptr); }

                const VkImage& get_image() const { return image; }

                const VkImageView& get_image_view() const { return image_view; }

            private:
                const vkb::DispatchTable& disp;
                VkImage image = {};
                VkImageView image_view = {};
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
                                              .build();

                    MAG_ASSERT(swap_ret, swap_ret.error().message() + " " + std::to_string(swap_ret.vk_result()));

                    vkb::destroy_swapchain(swapchain);

                    swapchain = swap_ret.value();

                    const std::vector<VkImage>& swapchain_images = swapchain.get_images().value();
                    const std::vector<VkImageView>& swapchain_image_views = swapchain.get_image_views().value();
                    for (u32 i = 0; i < swapchain.image_count; i++)
                    {
                        VulkanTexture* texture = new VulkanTexture(disp, swapchain_images[i], swapchain_image_views[i]);
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

        class VulkanGraphicsPipeline : public IGraphicsPipeline
        {
            public:
                VulkanGraphicsPipeline(const vkb::DispatchTable& disp, const IGraphicsPipelineDesc& desc) : disp(disp)
                {
                    Buffer vert_buffer;
                    mag::fs::read_binary_data(std::string(EXAMPLE_BUILD_DIRECTORY) + "/triangle.vert.spv", vert_buffer);
                    const auto vert_code = vert_buffer.data;

                    Buffer frag_buffer;
                    mag::fs::read_binary_data(std::string(EXAMPLE_BUILD_DIRECTORY) + "/triangle.frag.spv", frag_buffer);
                    const auto frag_code = frag_buffer.data;

                    VkShaderModule vert_module = createShaderModule(vert_code);
                    VkShaderModule frag_module = createShaderModule(frag_code);

                    if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE)
                    {
                        MAG_ASSERT(false, "Failed to create shader module");
                    }

                    VkPipelineShaderStageCreateInfo vert_stage_info = {};
                    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    vert_stage_info.module = vert_module;
                    vert_stage_info.pName = "main";

                    VkPipelineShaderStageCreateInfo frag_stage_info = {};
                    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    frag_stage_info.module = frag_module;
                    frag_stage_info.pName = "main";

                    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

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
                    pipeline_layout_info.setLayoutCount = 0;
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
                    pipeline_info.stageCount = 2;
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

                    disp.destroyShaderModule(frag_module, nullptr);
                    disp.destroyShaderModule(vert_module, nullptr);
                }

                ~VulkanGraphicsPipeline()
                {
                    disp.destroyPipeline(pipeline, nullptr);
                    disp.destroyPipelineLayout(pipeline_layout, nullptr);
                }

                // @TODO: temporary, this is here mostly for convenience
                VkShaderModule createShaderModule(const std::vector<u8>& code)
                {
                    VkShaderModuleCreateInfo create_info = {};
                    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    create_info.codeSize = code.size();
                    create_info.pCode = reinterpret_cast<const u32*>(code.data());

                    VkShaderModule shaderModule = {};
                    if (disp.createShaderModule(&create_info, nullptr, &shaderModule) != VK_SUCCESS)
                    {
                        return VK_NULL_HANDLE;  // failed to create shader module
                    }

                    return shaderModule;
                }

                const VkPipeline& get_pipeline() const { return pipeline; }

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

        class VulkanCommandBuffer : public ICommandBuffer
        {
            public:
                VulkanCommandBuffer(const vkb::DispatchTable& disp, const ICommandBufferDesc& desc,
                                    const VkCommandPool& pool)
                    : disp(disp), pool(pool)
                {
                    VkCommandBufferAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    alloc_info.commandPool = pool;
                    alloc_info.level = mag_to_vk(desc.command_buffer_level);
                    alloc_info.commandBufferCount = 1;

                    VK_CHECK(disp.allocateCommandBuffers(&alloc_info, &command_buffer),
                             "Failed to allocate command buffer");
                }

                ~VulkanCommandBuffer() { disp.freeCommandBuffers(pool, 1, &command_buffer); }

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

                virtual void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                                  const u32 first_instance) override
                {
                    disp.cmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
                }

                virtual void pipeline_barrier(const ITexture* texture, const u32 old_layout, const u32 new_layout,
                                              const u32 src_access_mask, const u32 dst_access_mask,
                                              const u32 src_stage_mask, const u32 dst_stage_mask) override
                {
                    VkImageMemoryBarrier image_memory_barrier = {};
                    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    image_memory_barrier.srcAccessMask = src_access_mask;
                    image_memory_barrier.dstAccessMask = dst_access_mask;
                    image_memory_barrier.oldLayout = (VkImageLayout)old_layout;
                    image_memory_barrier.newLayout = (VkImageLayout)new_layout;
                    image_memory_barrier.image = ((VulkanTexture*)texture)->get_image();
                    image_memory_barrier.subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    };

                    disp.cmdPipelineBarrier(command_buffer,
                                            src_stage_mask,  // srcStageMask
                                            dst_stage_mask,  // dstStageMask
                                            0, 0, nullptr, 0, nullptr,
                                            1,                     // imageMemoryBarrierCount
                                            &image_memory_barrier  // pImageMemoryBarriers
                    );
                }

                const VkCommandBuffer& get_command_buffer() const { return command_buffer; }

            private:
                const vkb::DispatchTable& disp;
                const VkCommandPool& pool;
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
                    submit_info.waitSemaphoreCount = 1;
                    submit_info.pWaitSemaphores = &((VulkanSemaphore*)wait_semaphore)->get_semaphore();
                    submit_info.pWaitDstStageMask = wait_stages;

                    submit_info.commandBufferCount = 1;
                    submit_info.pCommandBuffers = &((VulkanCommandBuffer*)command_buffer)->get_command_buffer();

                    submit_info.signalSemaphoreCount = 1;
                    submit_info.pSignalSemaphores = &((VulkanSemaphore*)signal_semaphore)->get_semaphore();

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

                    return disp.queuePresentKHR(queue, &present_info);
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

                    vkb::PhysicalDeviceSelector phys_device_selector(instance);
                    const auto phys_device_ret = phys_device_selector.set_minimum_version(1, 3)
                                                     .set_surface(surface)
                                                     .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                                                     .select();

                    MAG_ASSERT(phys_device_ret, phys_device_ret.error().message());

                    vkb::PhysicalDevice physical_device = phys_device_ret.value();
                    vkb::DeviceBuilder device_builder{physical_device};
                    const auto device_ret = device_builder.add_pNext(&dynamic_rendering_feature).build();

                    MAG_ASSERT(device_ret, device_ret.error().message());

                    device = device_ret.value();

                    disp = device.make_table();

                    // Swapchain
                    // -------------------------------------------------------------------------------------------------
                    ISwapchainDesc swapchain_desc = {};
                    swapchain_desc.desired_present_mode = PresentMode::Mailbox;
                    swapchain = this->create_swapchain(swapchain_desc);

                    // Queues
                    // -------------------------------------------------------------------------------------------------
                    graphics_queue = this->create_queue({.queue_type = QueueType::Graphics});
                    present_queue = this->create_queue({.queue_type = QueueType::Present});

                    // Graphics Pipeline
                    // -------------------------------------------------------------------------------------------------
                    IGraphicsPipelineDesc graphics_pipeline_desc = {};
                    graphics_pipeline_desc.primitive_topology = PrimitiveTopology::TriangleList;
                    graphics_pipeline_desc.format = swapchain->get_format();
                    graphics_pipeline_desc.extent = swapchain->get_extent();
                    graphics_pipeline = this->create_graphics_pipeline(graphics_pipeline_desc);

                    // Command Pool
                    // -------------------------------------------------------------------------------------------------
                    VkCommandPoolCreateInfo pool_info = {};
                    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    pool_info.queueFamilyIndex = device.get_queue_index(vkb::QueueType::graphics).value();

                    if (disp.createCommandPool(&pool_info, nullptr, &command_pool) != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create command pool");
                    }

                    // Command Buffers
                    // -------------------------------------------------------------------------------------------------
                    for (u32 i = 0; i < swapchain->get_image_count(); i++)
                    {
                        ICommandBufferDesc desc = {};
                        desc.command_buffer_level = CommandBufferLevel::Primary;
                        command_buffers.push_back(this->create_command_buffer(desc));
                    }

                    // Render Passes
                    // -------------------------------------------------------------------------------------------------
                    color_attachments.resize(command_buffers.size());
                    render_passes.resize(command_buffers.size());

                    for (u64 i = 0; i < command_buffers.size(); i++)
                    {
                        command_buffers[i]->begin_recording();

                        IRenderingAttachmentDesc color_attachment_desc = {};
                        color_attachment_desc.type = RenderingAttachmentType::Color;
                        color_attachment_desc.clear_color = {0.4f, 0.6f, 0.8f, 1.0f};
                        color_attachment_desc.texture = swapchain->get_texture(i);
                        color_attachments[i] = this->create_render_attachment(color_attachment_desc);

                        IRenderPassDesc render_pass_desc = {};
                        render_pass_desc.extent = swapchain->get_extent();
                        render_pass_desc.color_attachments.push_back(color_attachments[i].get());
                        render_passes[i] = this->create_render_pass(render_pass_desc);

                        command_buffers[i]->pipeline_barrier(swapchain->get_texture(i), VK_IMAGE_LAYOUT_UNDEFINED,
                                                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_NONE,
                                                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

                        command_buffers[i]->set_viewport(swapchain->get_extent());
                        command_buffers[i]->set_scissor(swapchain->get_extent());

                        command_buffers[i]->begin_rendering(render_passes[i].get());

                        command_buffers[i]->bind_pipeline(graphics_pipeline.get());

                        command_buffers[i]->draw(3);

                        command_buffers[i]->end_rendering();

                        command_buffers[i]->pipeline_barrier(
                            swapchain->get_texture(i), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_NONE,
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

                        command_buffers[i]->end_recording();
                    }

                    // Sync Objects
                    // -------------------------------------------------------------------------------------------------
                    available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
                    finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
                    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
                    image_in_flight.resize(swapchain->get_image_count());

                    IFenceDesc fence_desc = {};
                    fence_desc.signaled = true;

                    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                    {
                        available_semaphores[i] = this->create_semaphore();
                        finished_semaphore[i] = this->create_semaphore();
                        in_flight_fences[i] = this->create_fence(fence_desc);
                    }
                }

                ~VulkanDevice()
                {
                    disp.deviceWaitIdle();

                    in_flight_fences.clear();
                    available_semaphores.clear();
                    finished_semaphore.clear();

                    command_buffers.clear();

                    disp.destroyCommandPool(command_pool, nullptr);

                    graphics_pipeline.reset();
                    swapchain.reset();

                    vkb::destroy_device(device);
                    vkb::destroy_surface(instance, surface);
                    vkb::destroy_instance(instance);
                }

                virtual void draw_frame() override
                {
                    in_flight_fences[current_frame]->wait();

                    swapchain->acquire_next_image(available_semaphores[current_frame].get());
                    const u32 image_index = swapchain->get_current_image_index();

                    if (image_in_flight[image_index] != nullptr)
                    {
                        image_in_flight[image_index]->wait();
                    }
                    image_in_flight[image_index] = in_flight_fences[current_frame].get();

                    graphics_queue->submit(available_semaphores[current_frame].get(),
                                           finished_semaphore[current_frame].get(),
                                           in_flight_fences[current_frame].get(), command_buffers[image_index].get());

                    const VkResult result = static_cast<VkResult>(
                        present_queue->present(swapchain.get(), finished_semaphore[current_frame].get()));

                    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
                    {
                        MAG_ASSERT(false, "@TODO: resize swapchain");
                    }
                    else if (result != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to present swapchain image");
                    }

                    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
                }

                virtual unique<ISemaphore> create_semaphore() override { return create_unique<VulkanSemaphore>(disp); }

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

                virtual unique<ICommandBuffer> create_command_buffer(const ICommandBufferDesc& desc) override
                {
                    return create_unique<VulkanCommandBuffer>(disp, desc, command_pool);
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
                    return create_unique<VulkanTexture>(disp, desc);
                }

            private:
                vkb::Instance instance;
                vkb::Device device;
                vkb::InstanceDispatchTable inst_disp;
                vkb::DispatchTable disp;
                VkSurfaceKHR surface;
                VkCommandPool command_pool;
                unique<ISwapchain> swapchain;
                unique<IQueue> graphics_queue;
                unique<IQueue> present_queue;
                unique<IGraphicsPipeline> graphics_pipeline;
                std::vector<unique<ICommandBuffer>> command_buffers;
                std::vector<unique<IRenderPass>> render_passes;
                std::vector<unique<IRenderingAttachment>> color_attachments;
                std::vector<unique<ISemaphore>> available_semaphores;
                std::vector<unique<ISemaphore>> finished_semaphore;
                std::vector<unique<IFence>> in_flight_fences;
                std::vector<IFence*> image_in_flight;
                u32 current_frame = 0;
        };

        unique<IDevice> create_device() { return create_unique<VulkanDevice>(); }
    };  // namespace gfx
};      // namespace mag

#endif
