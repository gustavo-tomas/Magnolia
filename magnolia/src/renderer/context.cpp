#include "renderer/context.hpp"

#include <vulkan/vulkan_enums.hpp>

#include "core/logger.hpp"

namespace mag
{
    static Context* context = nullptr;

    Context& get_context()
    {
        ASSERT(context != nullptr, "Context is null");
        return *context;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void*)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            LOG_WARNING("{0}\n", pCallbackData->pMessage);

        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            LOG_ERROR("{0}\n", pCallbackData->pMessage);

        return VK_FALSE;
    }

    void Context::initialize(const ContextCreateOptions& options)
    {
        context = this;

        // Instance
        vk::ApplicationInfo app_info;
        app_info.setPApplicationName(options.application_name.c_str())
            .setApplicationVersion(VK_MAKE_API_VERSION(1, 0, 0, 0))
            .setPEngineName(options.engine_name.c_str())
            .setEngineVersion(VK_MAKE_API_VERSION(1, 0, 0, 0))
            .setApiVersion(VK_API_VERSION_1_3);

        std::vector<const char*> extensions = options.instance_extensions;
        std::vector<const char*> window_extensions = options.window.get_instance_extensions();

        extensions.insert(extensions.begin(), window_extensions.begin(), window_extensions.end());
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        vk::InstanceCreateInfo instance_create_info;
        instance_create_info.setPApplicationInfo(&app_info)
            .setPEnabledExtensionNames(extensions)
            .setPEnabledLayerNames(options.validation_layers);

        // Extensions
        LOG_INFO("Enumerating instance extensions");
        const auto extensions_properties = vk::enumerateInstanceExtensionProperties();
        for (const auto& extension_name : extensions)
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
        for (const auto& layer_name : options.validation_layers)
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

        this->api_version = app_info.apiVersion;

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

            if (properties.deviceType == options.preferred_device_type) break;
        }

        ASSERT(this->physical_device, "Failed to find suitable physical device");
        LOG_INFO("Selected physical device: {0}", str(physical_device.getProperties().deviceName));

        // Properties
        // const auto properties = this->physical_device.getProperties();
        // auto counts = properties.limits.framebufferColorSampleCounts &
        // properties.limits.framebufferDepthSampleCounts;

        // @TODO: hardcoded to 1 for now
        this->msaa_samples = vk::SampleCountFlagBits::e1;
        // if (counts & vk::SampleCountFlagBits::e2) this->msaa_samples = vk::SampleCountFlagBits::e2;
        // if (counts & vk::SampleCountFlagBits::e4) this->msaa_samples = vk::SampleCountFlagBits::e4;
        // if (counts & vk::SampleCountFlagBits::e8) this->msaa_samples = vk::SampleCountFlagBits::e8;
        // if (counts & vk::SampleCountFlagBits::e16) this->msaa_samples = vk::SampleCountFlagBits::e16;
        // if (counts & vk::SampleCountFlagBits::e32) this->msaa_samples = vk::SampleCountFlagBits::e32;
        // if (counts & vk::SampleCountFlagBits::e64) this->msaa_samples = vk::SampleCountFlagBits::e64;

        // Capabilities
        auto surface_present_modes = this->physical_device.getSurfacePresentModesKHR(this->surface);
        auto surface_capabilities = this->physical_device.getSurfaceCapabilitiesKHR(this->surface);
        auto surface_formats = this->physical_device.getSurfaceFormatsKHR(this->surface);

        this->present_image_count = max(surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
        ASSERT(this->present_image_count > 0, "Present image count must be greater than zero");
        LOG_INFO("Present image count: {0}", this->present_image_count);

        LOG_INFO("Enumerating surface present modes");
        this->surface_present_mode = vk::PresentModeKHR::eFifoRelaxed;
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

        vk::PhysicalDeviceShaderDrawParameterFeatures shader_draw_parameters_features;
        shader_draw_parameters_features.setShaderDrawParameters(true);

        vk::DeviceQueueCreateInfo device_queue_create_info;
        std::array queue_priorities = {1.0f};
        device_queue_create_info.setQueuePriorities(queue_priorities).setQueueFamilyIndex(this->queue_family_index);

        auto device_extensions = options.device_extensions;
        device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        // Logical device
        vk::DeviceCreateInfo device_create_info;
        device_create_info.setQueueCreateInfos(device_queue_create_info)
            .setPEnabledExtensionNames(device_extensions)
            .setPEnabledFeatures(&features)
            .setPNext(&shader_draw_parameters_features);

        this->device = this->physical_device.createDevice(device_create_info);
        this->graphics_queue = this->device.getQueue(this->queue_family_index, 0);

        ASSERT(this->device, "Failed to create device");

        this->dynamic_loader.init(this->instance, this->device);

        // Debug callback
        vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;
        debug_utils_messenger_create_info
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debug_callback);

        this->debug_utils_messenger = this->instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info,
                                                                                  nullptr, this->dynamic_loader);

        // Swapchain
        const uvec2 size(surface_capabilities.maxImageExtent.width, surface_capabilities.maxImageExtent.height);
        this->recreate_swapchain(size, this->surface_present_mode);

        // Sync structures
        this->upload_fence = this->device.createFence({});
        this->present_semaphore = this->device.createSemaphore({});
        this->render_semaphore = this->device.createSemaphore({});

        // Command pool
        vk::CommandPoolCreateInfo command_pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                    queue_family_index);
        this->command_pool = device.createCommandPool(command_pool_info);

        // Allocator
        // @TODO
        // VmaAllocatorCreateInfo allocator_create_info = {};
        // allocator_create_info.physicalDevice = this->physical_device;
        // allocator_create_info.device = this->device;
        // allocator_create_info.instance = this->instance;
        // allocator_create_info.vulkanApiVersion = this->api_version;

        // VK_CHECK(VK_CAST(vmaCreateAllocator(&allocator_create_info, &allocator)));

        this->command_buffer.initialize(command_pool, vk::CommandBufferLevel::ePrimary);
        this->frame_provider.initialize(options.frame_count);

        // @TODO
        // // Descriptors
        // descriptor_allocator.create(*this);
        // descriptor_cache.create(*this);
    }

    void Context::shutdown()
    {
        this->device.waitIdle();

        this->frame_provider.shutdown();

        for (const auto& image_view : swapchain_image_views) this->device.destroyImageView(image_view);

        this->device.destroyCommandPool(this->command_pool);
        this->device.destroyFence(this->upload_fence);
        this->device.destroySemaphore(this->present_semaphore);
        this->device.destroySemaphore(this->render_semaphore);
        this->device.destroySwapchainKHR(this->swapchain);
        this->device.destroy();

        this->instance.destroySurfaceKHR(this->surface);
        this->instance.destroyDebugUtilsMessengerEXT(this->debug_utils_messenger, nullptr, this->dynamic_loader);
        this->instance.destroy();
    }

    void Context::recreate_swapchain(const glm::uvec2& size, const vk::PresentModeKHR present_mode)
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

    void Context::end_frame() { this->frame_provider.end_frame(); }
};  // namespace mag
