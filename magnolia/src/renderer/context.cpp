#include "renderer/context.hpp"

#include "core/logger.hpp"

namespace mag
{
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
            bool available = false;
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
            bool available = false;
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
            LOG_INFO("Vendor: {0} (0x{1:X})", vendor_id_str(properties.vendorID), properties.vendorID);
            LOG_INFO("Type: {0}", physical_device_type_str.at(properties.deviceType));
            LOG_INFO("API Version: {0}", properties.apiVersion);
            if (properties.apiVersion < this->api_version) continue;

            u32 queue_family_index = 0;
            bool found_queue_family_index = false;
            const auto queue_family_properties = available_physical_device.getQueueFamilyProperties();
            for (const auto& property : queue_family_properties)
            {
                // @TODO: check for presentation support
                if ((property.queueCount > 0) &&
                    (available_physical_device.getSurfaceSupportKHR(queue_family_index, surface)) &&
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

        // Properties
        // const auto properties = this->physical_device.getProperties();
        // auto counts = properties.limits.framebufferColorSampleCounts &
        // properties.limits.framebufferDepthSampleCounts;

        // @TODO: MSAA
        // this->msaa_samples = vk::SampleCountFlagBits::e1;
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

        this->present_image_count = surface_capabilities.maxImageCount;

        this->surface_present_mode = vk::PresentModeKHR::eImmediate;
        if (std::find(surface_present_modes.begin(), surface_present_modes.end(), vk::PresentModeKHR::eMailbox) !=
            surface_present_modes.end())
        {
            this->surface_present_mode = vk::PresentModeKHR::eMailbox;
        }

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
    }

    void Context::shutdown()
    {
        this->device.destroy();
        this->instance.destroySurfaceKHR(this->surface);
        this->instance.destroyDebugUtilsMessengerEXT(this->debug_utils_messenger, nullptr, this->dynamic_loader);
        this->instance.destroy();
    }
};  // namespace mag
