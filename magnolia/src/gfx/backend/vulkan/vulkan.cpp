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
    #include "platform/file_system.hpp"

namespace mag
{
    // @TODO: temporary
    #define EXAMPLE_BUILD_DIRECTORY "magnolia/assets/shaders"
    #define MAX_FRAMES_IN_FLIGHT 3

    namespace gfx
    {
        class VkDevice : public IDevice
        {
            public:
                VkDevice()
                {
                    // Device
                    // -------------------------------------------------------------------------------------------------
                    vkb::InstanceBuilder instance_builder;
                    const auto instance_ret = instance_builder

    #if MAG_CONFIG_DEBUG
                                                  .use_default_debug_messenger()
                                                  .request_validation_layers()
    #endif
                                                  .build();

                    MAG_ASSERT(instance_ret, instance_ret.error().message());

                    instance = instance_ret.value();
                    inst_disp = instance.make_table();

                    window::create_surface(&instance.instance, &surface);

                    vkb::PhysicalDeviceSelector phys_device_selector(instance);
                    const auto phys_device_ret = phys_device_selector.set_surface(surface).select();

                    MAG_ASSERT(phys_device_ret, phys_device_ret.error().message());

                    vkb::PhysicalDevice physical_device = phys_device_ret.value();
                    vkb::DeviceBuilder device_builder{physical_device};
                    const auto device_ret = device_builder.build();

                    MAG_ASSERT(device_ret, device_ret.error().message());

                    device = device_ret.value();

                    disp = device.make_table();

                    // Swapchain
                    // -------------------------------------------------------------------------------------------------
                    vkb::SwapchainBuilder swapchain_builder{device};
                    const auto swap_ret = swapchain_builder.set_old_swapchain(swapchain).build();

                    MAG_ASSERT(swap_ret, swap_ret.error().message() + " " + std::to_string(swap_ret.vk_result()));

                    vkb::destroy_swapchain(swapchain);
                    swapchain = swap_ret.value();

                    // Queues
                    // -------------------------------------------------------------------------------------------------
                    const auto graphics_queue_ret = device.get_queue(vkb::QueueType::graphics);

                    MAG_ASSERT(graphics_queue_ret, graphics_queue_ret.error().message());

                    graphics_queue = graphics_queue_ret.value();

                    const auto present_queue_ret = device.get_queue(vkb::QueueType::present);

                    MAG_ASSERT(present_queue_ret.has_value(), present_queue_ret.error().message());

                    present_queue = present_queue_ret.value();

                    // RenderPass
                    // -------------------------------------------------------------------------------------------------
                    VkAttachmentDescription color_attachment = {};
                    color_attachment.format = swapchain.image_format;
                    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                    VkAttachmentReference color_attachment_ref = {};
                    color_attachment_ref.attachment = 0;
                    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    VkSubpassDescription subpass = {};
                    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpass.colorAttachmentCount = 1;
                    subpass.pColorAttachments = &color_attachment_ref;

                    VkSubpassDependency dependency = {};
                    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.dstSubpass = 0;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.srcAccessMask = 0;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstAccessMask =
                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    VkRenderPassCreateInfo render_pass_info = {};
                    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    render_pass_info.attachmentCount = 1;
                    render_pass_info.pAttachments = &color_attachment;
                    render_pass_info.subpassCount = 1;
                    render_pass_info.pSubpasses = &subpass;
                    render_pass_info.dependencyCount = 1;
                    render_pass_info.pDependencies = &dependency;

                    if (disp.createRenderPass(&render_pass_info, nullptr, &render_pass) != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create renderpass");
                    }

                    // Graphics Pipeline
                    // -------------------------------------------------------------------------------------------------
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
                    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                    input_assembly.primitiveRestartEnable = VK_FALSE;

                    VkViewport viewport = {};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = (float)swapchain.extent.width;
                    viewport.height = (float)swapchain.extent.height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;

                    VkRect2D scissor = {};
                    scissor.offset = {0, 0};
                    scissor.extent = swapchain.extent;

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

                    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
                    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                    colorBlendAttachment.blendEnable = VK_FALSE;

                    VkPipelineColorBlendStateCreateInfo color_blending = {};
                    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                    color_blending.logicOpEnable = VK_FALSE;
                    color_blending.logicOp = VK_LOGIC_OP_COPY;
                    color_blending.attachmentCount = 1;
                    color_blending.pAttachments = &colorBlendAttachment;
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
                    pipeline_info.renderPass = render_pass;
                    pipeline_info.subpass = 0;
                    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

                    if (disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) !=
                        VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to create pipeline");
                    }

                    disp.destroyShaderModule(frag_module, nullptr);
                    disp.destroyShaderModule(vert_module, nullptr);

                    // Framebuffers
                    // -------------------------------------------------------------------------------------------------
                    swapchain_images = swapchain.get_images().value();
                    swapchain_image_views = swapchain.get_image_views().value();

                    framebuffers.resize(swapchain_image_views.size());

                    for (u64 i = 0; i < swapchain_image_views.size(); i++)
                    {
                        VkImageView attachments[] = {swapchain_image_views[i]};

                        VkFramebufferCreateInfo framebuffer_info = {};
                        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                        framebuffer_info.renderPass = render_pass;
                        framebuffer_info.attachmentCount = 1;
                        framebuffer_info.pAttachments = attachments;
                        framebuffer_info.width = swapchain.extent.width;
                        framebuffer_info.height = swapchain.extent.height;
                        framebuffer_info.layers = 1;

                        if (disp.createFramebuffer(&framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS)
                        {
                            MAG_ASSERT(false, "Failed to create framebuffer");
                        }
                    }

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
                    command_buffers.resize(framebuffers.size());

                    VkCommandBufferAllocateInfo allocInfo = {};
                    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    allocInfo.commandPool = command_pool;
                    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    allocInfo.commandBufferCount = (u32)command_buffers.size();

                    if (disp.allocateCommandBuffers(&allocInfo, command_buffers.data()) != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to allocate command buffers");
                    }

                    for (u64 i = 0; i < command_buffers.size(); i++)
                    {
                        VkCommandBufferBeginInfo begin_info = {};
                        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                        if (disp.beginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
                        {
                            MAG_ASSERT(false, "Failed to begin command buffer recording");
                        }

                        VkRenderPassBeginInfo render_pass_info = {};
                        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                        render_pass_info.renderPass = render_pass;
                        render_pass_info.framebuffer = framebuffers[i];
                        render_pass_info.renderArea.offset = {0, 0};
                        render_pass_info.renderArea.extent = swapchain.extent;

                        VkClearValue clearColor{{{0.4f, 0.6f, 0.8f, 1.0f}}};
                        render_pass_info.clearValueCount = 1;
                        render_pass_info.pClearValues = &clearColor;

                        VkViewport viewport = {};
                        viewport.x = 0.0f;
                        viewport.y = 0.0f;
                        viewport.width = (float)swapchain.extent.width;
                        viewport.height = (float)swapchain.extent.height;
                        viewport.minDepth = 0.0f;
                        viewport.maxDepth = 1.0f;

                        VkRect2D scissor = {};
                        scissor.offset = {0, 0};
                        scissor.extent = swapchain.extent;

                        disp.cmdSetViewport(command_buffers[i], 0, 1, &viewport);
                        disp.cmdSetScissor(command_buffers[i], 0, 1, &scissor);

                        disp.cmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

                        disp.cmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

                        disp.cmdDraw(command_buffers[i], 3, 1, 0, 0);

                        disp.cmdEndRenderPass(command_buffers[i]);

                        if (disp.endCommandBuffer(command_buffers[i]) != VK_SUCCESS)
                        {
                            MAG_ASSERT(false, "Failed to record command buffer");
                        }
                    }

                    // Sync Objects
                    // -------------------------------------------------------------------------------------------------
                    available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
                    finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
                    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
                    image_in_flight.resize(swapchain.image_count, VK_NULL_HANDLE);

                    VkSemaphoreCreateInfo semaphore_info = {};
                    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                    VkFenceCreateInfo fence_info = {};
                    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                    {
                        if (disp.createSemaphore(&semaphore_info, nullptr, &available_semaphores[i]) != VK_SUCCESS ||
                            disp.createSemaphore(&semaphore_info, nullptr, &finished_semaphore[i]) != VK_SUCCESS ||
                            disp.createFence(&fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
                        {
                            MAG_ASSERT(false, "Failed to sync objects");
                        }
                    }
                }

                ~VkDevice()
                {
                    disp.deviceWaitIdle();

                    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                    {
                        disp.destroySemaphore(finished_semaphore[i], nullptr);
                        disp.destroySemaphore(available_semaphores[i], nullptr);
                        disp.destroyFence(in_flight_fences[i], nullptr);
                    }

                    disp.destroyCommandPool(command_pool, nullptr);

                    for (auto framebuffer : framebuffers)
                    {
                        disp.destroyFramebuffer(framebuffer, nullptr);
                    }

                    disp.destroyPipeline(graphics_pipeline, nullptr);
                    disp.destroyPipelineLayout(pipeline_layout, nullptr);
                    disp.destroyRenderPass(render_pass, nullptr);

                    swapchain.destroy_image_views(swapchain_image_views);

                    vkb::destroy_swapchain(swapchain);
                    vkb::destroy_device(device);
                    vkb::destroy_surface(instance, surface);
                    vkb::destroy_instance(instance);
                }

                virtual void draw_frame() override
                {
                    disp.waitForFences(1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

                    u32 image_index = 0;
                    VkResult result = disp.acquireNextImageKHR(
                        swapchain, UINT64_MAX, available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

                    if (result == VK_ERROR_OUT_OF_DATE_KHR)
                    {
                        // return recreate_swapchain(init, data);
                        MAG_ASSERT(false, "@TODO: resize swapchain");
                    }
                    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                    {
                        MAG_ASSERT(false, "Failed to acquire swapchain image");
                    }

                    if (image_in_flight[image_index] != VK_NULL_HANDLE)
                    {
                        disp.waitForFences(1, &image_in_flight[image_index], VK_TRUE, UINT64_MAX);
                    }
                    image_in_flight[image_index] = in_flight_fences[current_frame];

                    VkSubmitInfo submitInfo = {};
                    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                    VkSemaphore wait_semaphores[] = {available_semaphores[current_frame]};
                    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                    submitInfo.waitSemaphoreCount = 1;
                    submitInfo.pWaitSemaphores = wait_semaphores;
                    submitInfo.pWaitDstStageMask = wait_stages;

                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &command_buffers[image_index];

                    VkSemaphore signal_semaphores[] = {finished_semaphore[current_frame]};
                    submitInfo.signalSemaphoreCount = 1;
                    submitInfo.pSignalSemaphores = signal_semaphores;

                    disp.resetFences(1, &in_flight_fences[current_frame]);

                    if (disp.queueSubmit(graphics_queue, 1, &submitInfo, in_flight_fences[current_frame]) != VK_SUCCESS)
                    {
                        MAG_ASSERT(false, "Failed to submit draw command buffer");
                    }

                    VkPresentInfoKHR present_info = {};
                    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                    present_info.waitSemaphoreCount = 1;
                    present_info.pWaitSemaphores = signal_semaphores;

                    VkSwapchainKHR swapChains[] = {swapchain};
                    present_info.swapchainCount = 1;
                    present_info.pSwapchains = swapChains;

                    present_info.pImageIndices = &image_index;

                    result = disp.queuePresentKHR(present_queue, &present_info);
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

                VkShaderModule createShaderModule(const std::vector<u8>& code)
                {
                    VkShaderModuleCreateInfo create_info = {};
                    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    create_info.codeSize = code.size();
                    create_info.pCode = reinterpret_cast<const u32*>(code.data());

                    VkShaderModule shaderModule;
                    if (disp.createShaderModule(&create_info, nullptr, &shaderModule) != VK_SUCCESS)
                    {
                        return VK_NULL_HANDLE;  // failed to create shader module
                    }

                    return shaderModule;
                }

            private:
                vkb::Instance instance;
                vkb::Device device;
                vkb::Swapchain swapchain;
                vkb::InstanceDispatchTable inst_disp;
                vkb::DispatchTable disp;
                VkSurfaceKHR surface;
                VkQueue graphics_queue;
                VkQueue present_queue;
                VkRenderPass render_pass;
                VkPipelineLayout pipeline_layout;
                VkPipeline graphics_pipeline;
                std::vector<VkImage> swapchain_images;
                std::vector<VkImageView> swapchain_image_views;
                std::vector<VkFramebuffer> framebuffers;
                VkCommandPool command_pool;
                std::vector<VkCommandBuffer> command_buffers;
                std::vector<VkSemaphore> available_semaphores;
                std::vector<VkSemaphore> finished_semaphore;
                std::vector<VkFence> in_flight_fences;
                std::vector<VkFence> image_in_flight;
                u32 current_frame = 0;
        };

        unique<IDevice> create_device() { return create_unique<VkDevice>(); }
    };  // namespace gfx
};      // namespace mag

#endif
