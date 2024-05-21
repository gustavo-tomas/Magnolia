#include "renderer/context.hpp"

#include "core/logger.hpp"
#include "core/types.hpp"

// Use to trace VMA allocations
#ifdef MAG_DEBUG_TRACE
    #define VMA_DEBUG_LOG_FORMAT(format, ...) \
        do                                    \
        {                                     \
            printf((format), __VA_ARGS__);    \
            printf("\n");                     \
        } while (false)

    #define VMA_DEBUG_LOG(str) VMA_DEBUG_LOG_FORMAT("%s", (str))
#endif

#define VMA_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#include "vk_mem_alloc.h"
#pragma GCC diagnostic pop

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mag
{
    static Context* context = nullptr;

    Context& get_context()
    {
        ASSERT(context != nullptr, "Context is null");
        return *context;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* userData)
    {
        (void)messageType;
        (void)userData;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARNING("{0}\n", pCallbackData->pMessage);
        }

        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("{0}\n", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    Context::Context(const ContextCreateOptions& options)
    {
        context = this;

        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        std::vector<const char*> instance_extensions;
        std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};
        std::vector<const char*> validation_layers;

        const vk::PhysicalDeviceType preferred_device_type = vk::PhysicalDeviceType::eDiscreteGpu;
        const u32 api_version = VK_API_VERSION_1_3;
        const u32 frame_count = 3;  // 3 for triple buffering

        const std::vector<const char*> window_extensions = options.window.get_instance_extensions();
        instance_extensions.insert(instance_extensions.begin(), window_extensions.begin(), window_extensions.end());

        // Validation only on debug
#if defined(MAG_DEBUG)
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        validation_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        // Instance
        vk::ApplicationInfo app_info;
        app_info.setPApplicationName(options.application_name.c_str())
            .setApplicationVersion(VK_MAKE_API_VERSION(1, 0, 0, 0))
            .setPEngineName(options.engine_name.c_str())
            .setEngineVersion(VK_MAKE_API_VERSION(1, 0, 0, 0))
            .setApiVersion(api_version);

        vk::InstanceCreateInfo instance_create_info;
        instance_create_info.setPApplicationInfo(&app_info)
            .setPEnabledExtensionNames(instance_extensions)
            .setPEnabledLayerNames(validation_layers);

        // Extensions
        LOG_INFO("Enumerating instance extensions");
        const auto extensions_properties = vk::enumerateInstanceExtensionProperties();
        for (const auto& extension_name : instance_extensions)
        {
            LOG_INFO("Extension: {0}", extension_name);
            b8 available = false;
            for (const auto& extension_property : extensions_properties)
            {
                if (std::strcmp(extension_property.extensionName.data(), extension_name) == 0)
                {
                    available = true;
                    break;
                }
            }

            ASSERT(available, "Extension not available: " + str(extension_name));
        }

        // Validation layers
        LOG_INFO("Enumerating instance layer properties");
        const auto layer_properties = vk::enumerateInstanceLayerProperties();
        for (const auto& layer_name : validation_layers)
        {
            LOG_INFO("Layer: {0}", layer_name);
            b8 available = false;
            for (const auto& layer_property : layer_properties)
            {
                if (std::strcmp(layer_property.layerName.data(), layer_name) == 0)
                {
                    available = true;
                    break;
                }
            }

            ASSERT(available, "Layer not available: " + str(layer_name));
        }

        VK_CHECK(vk::createInstance(&instance_create_info, nullptr, &instance));

        VULKAN_HPP_DEFAULT_DISPATCHER.init(this->instance);

        this->api_version = app_info.apiVersion;
        this->frame_count = frame_count;

        // Surface
        this->surface = options.window.create_surface(this->instance);

        // Physical device
        LOG_INFO("Enumerating physical devices");
        const auto available_physical_devices = this->instance.enumeratePhysicalDevices();
        for (const auto& available_physical_device : available_physical_devices)
        {
            const auto properties = available_physical_device.getProperties();
            LOG_INFO("Device: {0}", str(properties.deviceName));
            LOG_INFO("Vendor: 0x{0:X}", properties.vendorID);
            LOG_INFO("Type: {0}", vk::to_string(properties.deviceType));
            LOG_INFO("API Version: {0}", properties.apiVersion);
            if (properties.apiVersion < this->api_version) continue;

            u32 queue_family_index = 0;
            b8 found_queue_family_index = false;
            const auto queue_family_properties = available_physical_device.getQueueFamilyProperties();
            for (const auto& property : queue_family_properties)
            {
                if ((property.queueCount > 0) &&
                    (available_physical_device.getSurfaceSupportKHR(queue_family_index, surface)) &&
                    (available_physical_device.getSurfacePresentModesKHR(surface).empty() == false) &&
                    (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
                    (property.queueFlags & vk::QueueFlagBits::eCompute))
                {
                    found_queue_family_index = true;
                    break;
                }

                queue_family_index++;
            }
            if (!found_queue_family_index) continue;

            this->physical_device = available_physical_device;
            this->queue_family_index = queue_family_index;

            if (properties.deviceType == preferred_device_type) break;
        }

        ASSERT(this->physical_device, "Failed to find suitable physical device");
        LOG_INFO("Selected physical device: {0}", str(physical_device.getProperties().deviceName));

        // Properties
        physical_device_properties.pNext = &descriptor_buffer_properties;
        physical_device.getProperties2(&physical_device_properties);

        // @TODO: harcoded to max samples
        const auto properties = this->physical_device_properties.properties;
        const auto counts = properties.limits.framebufferColorSampleCounts &
                            properties.limits.framebufferDepthSampleCounts &
                            properties.limits.framebufferStencilSampleCounts;

        this->msaa_samples = (counts & vk::SampleCountFlagBits::e64)   ? vk::SampleCountFlagBits::e64
                             : (counts & vk::SampleCountFlagBits::e32) ? vk::SampleCountFlagBits::e32
                             : (counts & vk::SampleCountFlagBits::e16) ? vk::SampleCountFlagBits::e16
                             : (counts & vk::SampleCountFlagBits::e8)  ? vk::SampleCountFlagBits::e8
                             : (counts & vk::SampleCountFlagBits::e4)  ? vk::SampleCountFlagBits::e4
                             : (counts & vk::SampleCountFlagBits::e2)  ? vk::SampleCountFlagBits::e2
                                                                       : vk::SampleCountFlagBits::e1;

        // Capabilities
        auto surface_present_modes = this->physical_device.getSurfacePresentModesKHR(this->surface);
        auto surface_capabilities = this->physical_device.getSurfaceCapabilitiesKHR(this->surface);
        auto surface_formats = this->physical_device.getSurfaceFormatsKHR(this->surface);

        this->present_image_count = max(surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
        ASSERT(this->present_image_count > 0, "Present image count must be greater than zero");
        LOG_INFO("Present image count: {0}", this->present_image_count);

        LOG_INFO("Enumerating surface present modes");
        this->surface_present_mode = vk::PresentModeKHR::eImmediate;
        for (const auto& present_mode : surface_present_modes)
        {
            LOG_INFO("Present mode: {0}", vk::to_string(present_mode));
            if (present_mode == vk::PresentModeKHR::eMailbox) this->surface_present_mode = present_mode;
        }
        LOG_INFO("Selected present mode: {0}", vk::to_string(this->surface_present_mode));

        this->surface_format = surface_formats.front();
        for (const auto& format : surface_formats)
            if (format.format == vk::Format::eR8G8B8A8Srgb || format.format == vk::Format::eB8G8R8A8Srgb)
                this->surface_format = format;

        // Features
        vk::PhysicalDeviceFeatures features;
        features.setSamplerAnisotropy(true);

        vk::PhysicalDeviceSynchronization2FeaturesKHR synchronization_2_features(true);

        vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features({});
        descriptor_indexing_features.setDescriptorBindingVariableDescriptorCount(true);
        descriptor_indexing_features.setPNext(&synchronization_2_features);

        vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_device_address_features(true, {}, {},
                                                                                        &descriptor_indexing_features);
        vk::PhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_features(true, {}, {}, {},
                                                                                 &buffer_device_address_features);
        vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features(true, &descriptor_buffer_features);
        vk::PhysicalDeviceShaderDrawParameterFeatures shader_draw_parameters_features(true,
                                                                                      &dynamic_rendering_features);

        vk::DeviceQueueCreateInfo device_queue_create_info;
        std::array queue_priorities = {1.0f};
        device_queue_create_info.setQueuePriorities(queue_priorities).setQueueFamilyIndex(this->queue_family_index);

        LOG_INFO("Enumerating device extension properties");
        const auto device_properties = this->physical_device.enumerateDeviceExtensionProperties();
        for (const auto& device_extension : device_extensions)
        {
            LOG_INFO("Extension: {0}", device_extension);
            b8 available = false;
            for (const auto& device_property : device_properties)
            {
                if (std::strcmp(device_property.extensionName.data(), device_extension) == 0)
                {
                    available = true;
                    break;
                }
            }

            ASSERT(available, "Extension not available: " + str(device_extension));
        }

        // Logical device
        vk::DeviceCreateInfo device_create_info;
        device_create_info.setQueueCreateInfos(device_queue_create_info)
            .setPEnabledExtensionNames(device_extensions)
            .setPEnabledFeatures(&features)
            .setPNext(&shader_draw_parameters_features);

        this->device = this->physical_device.createDevice(device_create_info);
        ASSERT(this->device, "Failed to create device");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(this->device);

        this->graphics_queue = this->device.getQueue(this->queue_family_index, 0);

        // Debug callback
#if defined(MAG_DEBUG)
        vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;
        debug_utils_messenger_create_info
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debug_callback);

        this->debug_utils_messenger = this->instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
#endif

        // Swapchain
        const uvec2 size(surface_capabilities.maxImageExtent.width, surface_capabilities.maxImageExtent.height);
        this->recreate_swapchain(size, this->surface_present_mode);

        // Fence
        this->upload_fence = device.createFence({});

        // Command pool
        vk::CommandPoolCreateInfo command_pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                    queue_family_index);
        this->command_pool = device.createCommandPool(command_pool_info);

        // Command buffer
        this->submit_command_buffer.initialize(this->command_pool, vk::CommandBufferLevel::ePrimary);

        // Allocator
        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.physicalDevice = this->physical_device;
        allocator_create_info.device = this->device;
        allocator_create_info.instance = this->instance;
        allocator_create_info.vulkanApiVersion = this->api_version;
        allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        VK_CHECK(VK_CAST(vmaCreateAllocator(&allocator_create_info, &allocator)));

        this->frame_provider.initialize(frame_count);

        // Descriptors
        descriptor_layout_cache.initialize();
        descriptor_cache = std::make_unique<DescriptorCache>();
    }

    Context::~Context()
    {
        this->device.waitIdle();

        this->descriptor_cache.reset();

        vmaDestroyAllocator(this->allocator);

        this->descriptor_layout_cache.shutdown();
        this->frame_provider.shutdown();

        for (const auto& image_view : swapchain_image_views) this->device.destroyImageView(image_view);

        this->device.destroyFence(this->upload_fence);
        this->device.destroyCommandPool(this->command_pool);
        this->device.destroySwapchainKHR(this->swapchain);
        this->device.destroy();

        this->instance.destroySurfaceKHR(this->surface);
#if defined(MAG_DEBUG)
        this->instance.destroyDebugUtilsMessengerEXT(this->debug_utils_messenger);
#endif
        this->instance.destroy();
    }

    void Context::recreate_swapchain(const uvec2& size) { this->recreate_swapchain(size, this->surface_present_mode); }

    void Context::recreate_swapchain(const uvec2& size, const vk::PresentModeKHR present_mode)
    {
        this->device.waitIdle();

        auto surface_capabilities = this->physical_device.getSurfaceCapabilitiesKHR(this->surface);

        this->surface_extent = vk::Extent2D(
            std::clamp(size.x, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
            std::clamp(size.y, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height));

        vk::SwapchainCreateInfoKHR swapchain_create_info(
            {}, this->surface, this->present_image_count, this->surface_format.format, this->surface_format.colorSpace,
            this->surface_extent, 1, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive, {}, surface_capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque, present_mode, true, this->swapchain);

        this->swapchain = this->device.createSwapchainKHR(swapchain_create_info);

        // Destroy old swapchain
        vk::SwapchainKHR old_swapchain = swapchain_create_info.oldSwapchain;
        if (old_swapchain) this->device.destroySwapchainKHR(old_swapchain);
        for (const auto& image_view : swapchain_image_views) this->device.destroyImageView(image_view);
        this->swapchain_image_views.clear();

        // Create new swapchain
        this->swapchain_images = this->device.getSwapchainImagesKHR(this->swapchain);
        this->present_image_count = VECSIZE(this->swapchain_images);
        this->surface_present_mode = present_mode;

        this->swapchain_image_views.reserve(this->present_image_count);
        for (const auto& image : this->swapchain_images)
        {
            const vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            const vk::ImageViewCreateInfo image_view_create_info({}, image, vk::ImageViewType::e2D,
                                                                 this->surface_format.format, {}, range);

            this->swapchain_image_views.push_back(device.createImageView(image_view_create_info));
        }
    }

    void Context::begin_frame() { this->frame_provider.begin_frame(); }

    b8 Context::end_frame(const Image& image, const vk::Extent3D& extent)
    {
        return this->frame_provider.end_frame(image, extent);
    }

    void Context::submit_commands_immediate(std::function<void(CommandBuffer cmd)>&& function)
    {
        auto& cmd = this->submit_command_buffer;

        cmd.begin();
        function(cmd);  // execute the function
        cmd.end();

        vk::SubmitInfo submit;
        submit.setCommandBuffers(cmd.get_handle());

        this->graphics_queue.submit(submit, this->upload_fence);
        VK_CHECK(this->device.waitForFences(this->upload_fence, true, MAG_TIMEOUT));
        this->device.resetFences(this->upload_fence);
        this->device.resetCommandPool(command_pool);
    }

    vk::Format Context::get_supported_format(const std::vector<vk::Format>& candidates, const vk::ImageTiling tiling,
                                             const vk::FormatFeatureFlags features) const
    {
        for (const auto& format : candidates)
        {
            const vk::FormatProperties properties = physical_device.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
            {
                return format;
            }

            else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        ASSERT(false, "Failed to get supported format for Tiling: " + vk::to_string(tiling) +
                          ", Features: " + vk::to_string(features));
    }

    vk::Format Context::get_supported_depth_format() const
    {
        return this->get_supported_format(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
};  // namespace mag
