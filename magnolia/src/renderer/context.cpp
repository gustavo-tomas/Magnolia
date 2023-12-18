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

    void Context::create_instance(const ContextCreateOptions& options)
    {
        vk::ApplicationInfo app_info;
        app_info.setPApplicationName(options.application_name.c_str())
            .setApplicationVersion(VK_MAKE_API_VERSION(1, 0, 0, 0))
            .setPEngineName(options.engine_name.c_str())
            .setEngineVersion(VK_MAKE_API_VERSION(1, 0, 0, 0))
            .setApiVersion(VK_API_VERSION_1_3);

        std::vector<const char*> extensions = options.extensions;
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        vk::InstanceCreateInfo instance_create_info;
        instance_create_info.setPApplicationInfo(&app_info)
            .setPEnabledExtensionNames(extensions)
            .setPEnabledLayerNames(options.layers);

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
        for (const auto& layer_name : options.layers)
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
    }

    void Context::destroy() { this->instance.destroy(); }
};  // namespace mag
