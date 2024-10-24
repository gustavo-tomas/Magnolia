#include "renderer/context.hpp"

#include "core/debug.hpp"
#include "core/logger.hpp"
#include "core/window.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/frame.hpp"
#include "tools/profiler.hpp"

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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

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

        DEBUG_BREAK();
        return VK_FALSE;
    }

    struct Context::IMPL
    {
            vk::Instance instance;
            vk::SurfaceKHR surface;
            vk::SurfaceFormatKHR surface_format;
            vk::Extent2D surface_extent;
            vk::PhysicalDevice physical_device;
            vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties;
            vk::PhysicalDeviceProperties2 physical_device_properties;
            vk::Device device;
            vk::PresentModeKHR surface_present_mode;
            vk::SwapchainKHR swapchain;
            vk::Queue graphics_queue;
            vk::CommandPool command_pool, immediate_command_pool;
            vk::Fence upload_fence;
            vk::DebugUtilsMessengerEXT debug_utils_messenger;
            vk::QueryPool query_pool;
            vk::SampleCountFlagBits msaa_samples;

            std::vector<vk::Image> swapchain_images;
            std::vector<vk::ImageView> swapchain_image_views;

            u32 api_version = {};
            u32 queue_family_index = {};
            u32 present_image_count = {};
            u32 frame_count = {};
            const u32 query_count = 2;
            b8 is_query_supported = {};
            f32 timestamp_period = {};
            ProfileResult timestamp = {};

            FrameProvider frame_provider;
            VmaAllocator allocator = {};
            unique<DescriptorLayoutCache> descriptor_layout_cache;
            unique<DescriptorAllocator> descriptor_allocator;

            CommandBuffer submit_command_buffer;
    };

    Context::Context(const ContextCreateOptions& options) : impl(new IMPL())
    {
        context = this;

        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        std::vector<const c8*> instance_extensions;
        std::vector<const c8*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};
        std::vector<const c8*> validation_layers;

        const vk::PhysicalDeviceType preferred_device_type = vk::PhysicalDeviceType::eDiscreteGpu;
        const u32 api_version = VK_API_VERSION_1_3;
        const u32 frame_count = 3;  // 3 for triple buffering

        const std::vector<const c8*> window_extensions = options.window.get_instance_extensions();
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

        VK_CHECK(vk::createInstance(&instance_create_info, nullptr, &impl->instance));

        VULKAN_HPP_DEFAULT_DISPATCHER.init(impl->instance);

        impl->api_version = app_info.apiVersion;
        impl->frame_count = frame_count;

        // Surface
        options.window.create_surface(&impl->instance, &impl->surface);

        // Physical device
        LOG_INFO("Enumerating physical devices");
        const auto available_physical_devices = impl->instance.enumeratePhysicalDevices();
        for (const auto& available_physical_device : available_physical_devices)
        {
            const auto properties = available_physical_device.getProperties();
            LOG_INFO("Device: {0}", str(properties.deviceName));
            LOG_INFO("Vendor: 0x{0:X}", properties.vendorID);
            LOG_INFO("Type: {0}", vk::to_string(properties.deviceType));
            LOG_INFO("API Version: {0}", properties.apiVersion);
            if (properties.apiVersion < impl->api_version) continue;

            u32 queue_family_index = 0;
            b8 found_queue_family_index = false;
            const auto queue_family_properties = available_physical_device.getQueueFamilyProperties();
            for (const auto& property : queue_family_properties)
            {
                if ((property.queueCount > 0) &&
                    (available_physical_device.getSurfaceSupportKHR(queue_family_index, impl->surface)) &&
                    (available_physical_device.getSurfacePresentModesKHR(impl->surface).empty() == false) &&
                    (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
                    (property.queueFlags & vk::QueueFlagBits::eCompute))
                {
                    found_queue_family_index = true;
                    break;
                }

                queue_family_index++;
            }
            if (!found_queue_family_index) continue;

            impl->physical_device = available_physical_device;
            impl->queue_family_index = queue_family_index;

            if (properties.deviceType == preferred_device_type) break;
        }

        ASSERT(impl->physical_device, "Failed to find suitable physical device");
        LOG_INFO("Selected physical device: {0}", str(impl->physical_device.getProperties().deviceName));

        // Properties
        impl->physical_device_properties.pNext = &impl->descriptor_buffer_properties;
        impl->physical_device.getProperties2(&impl->physical_device_properties);

        // Query support
        impl->is_query_supported = impl->physical_device_properties.properties.limits.timestampComputeAndGraphics;

        // @TODO: harcoded to max samples
        const auto properties = impl->physical_device_properties.properties;
        const auto counts = properties.limits.framebufferColorSampleCounts &
                            properties.limits.framebufferDepthSampleCounts &
                            properties.limits.framebufferStencilSampleCounts;

        impl->msaa_samples = (counts & vk::SampleCountFlagBits::e64)   ? vk::SampleCountFlagBits::e64
                             : (counts & vk::SampleCountFlagBits::e32) ? vk::SampleCountFlagBits::e32
                             : (counts & vk::SampleCountFlagBits::e16) ? vk::SampleCountFlagBits::e16
                             : (counts & vk::SampleCountFlagBits::e8)  ? vk::SampleCountFlagBits::e8
                             : (counts & vk::SampleCountFlagBits::e4)  ? vk::SampleCountFlagBits::e4
                             : (counts & vk::SampleCountFlagBits::e2)  ? vk::SampleCountFlagBits::e2
                                                                       : vk::SampleCountFlagBits::e1;

        // Capabilities
        auto surface_present_modes = impl->physical_device.getSurfacePresentModesKHR(impl->surface);
        auto surface_capabilities = impl->physical_device.getSurfaceCapabilitiesKHR(impl->surface);
        auto surface_formats = impl->physical_device.getSurfaceFormatsKHR(impl->surface);

        impl->present_image_count = max(surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
        ASSERT(impl->present_image_count > 0, "Present image count must be greater than zero");
        LOG_INFO("Present image count: {0}", impl->present_image_count);

        LOG_INFO("Enumerating surface present modes");
        impl->surface_present_mode = vk::PresentModeKHR::eImmediate;
        for (const auto& present_mode : surface_present_modes)
        {
            LOG_INFO("Present mode: {0}", vk::to_string(present_mode));
            if (present_mode == vk::PresentModeKHR::eMailbox) impl->surface_present_mode = present_mode;
        }
        LOG_INFO("Selected present mode: {0}", vk::to_string(impl->surface_present_mode));

        impl->surface_format = surface_formats.front();
        for (const auto& format : surface_formats)
            if (format.format == vk::Format::eR8G8B8A8Srgb || format.format == vk::Format::eB8G8R8A8Srgb)
                impl->surface_format = format;

        // Features
        vk::PhysicalDeviceFeatures features;
        features.setSamplerAnisotropy(true);
        features.setFillModeNonSolid(true);

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
        device_queue_create_info.setQueuePriorities(queue_priorities).setQueueFamilyIndex(impl->queue_family_index);

        LOG_INFO("Enumerating device extension properties");
        const auto device_properties = impl->physical_device.enumerateDeviceExtensionProperties();
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

        impl->device = impl->physical_device.createDevice(device_create_info);
        ASSERT(impl->device, "Failed to create device");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(impl->device);

        impl->graphics_queue = impl->device.getQueue(impl->queue_family_index, 0);

        // Debug callback
#if defined(MAG_DEBUG)
        vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;
        debug_utils_messenger_create_info
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debug_callback);

        impl->debug_utils_messenger = impl->instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
#endif

        // Swapchain
        const uvec2 size(surface_capabilities.maxImageExtent.width, surface_capabilities.maxImageExtent.height);
        recreate_swapchain(size, impl->surface_present_mode);

        // Fence
        impl->upload_fence = impl->device.createFence({});

        // Command pool
        const vk::CommandPoolCreateInfo command_pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                          impl->queue_family_index);
        impl->command_pool = impl->device.createCommandPool(command_pool_info);
        impl->immediate_command_pool = impl->device.createCommandPool(command_pool_info);

        // Command buffer
        impl->submit_command_buffer.initialize(impl->immediate_command_pool, vk::CommandBufferLevel::ePrimary);

        // Allocator
        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.physicalDevice = impl->physical_device;
        allocator_create_info.device = impl->device;
        allocator_create_info.instance = impl->instance;
        allocator_create_info.vulkanApiVersion = impl->api_version;
        allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        VK_CHECK(VK_CAST(vmaCreateAllocator(&allocator_create_info, &impl->allocator)));

        impl->frame_provider.initialize(frame_count);

        // Query pool
        if (impl->is_query_supported)
        {
            vk::QueryPoolCreateInfo query_pool_info({}, vk::QueryType::eTimestamp, impl->query_count);
            impl->query_pool = impl->device.createQueryPool(query_pool_info);
            impl->timestamp_period = properties.limits.timestampPeriod;
        }

        else
        {
            LOG_WARNING("Timestamp query is not suppported on this device");
        }

        // Descriptors
        impl->descriptor_layout_cache = create_unique<DescriptorLayoutCache>();
        impl->descriptor_allocator = create_unique<DescriptorAllocator>();
    }

    Context::~Context()
    {
        impl->device.waitIdle();

        impl->descriptor_layout_cache.reset();
        impl->descriptor_allocator.reset();

        vmaDestroyAllocator(impl->allocator);

        impl->frame_provider.shutdown();

        for (const auto& image_view : impl->swapchain_image_views) impl->device.destroyImageView(image_view);

        impl->device.destroyQueryPool(impl->query_pool);
        impl->device.destroyFence(impl->upload_fence);
        impl->device.destroyCommandPool(impl->command_pool);
        impl->device.destroyCommandPool(impl->immediate_command_pool);
        impl->device.destroySwapchainKHR(impl->swapchain);
        impl->device.destroy();

        impl->instance.destroySurfaceKHR(impl->surface);
#if defined(MAG_DEBUG)
        impl->instance.destroyDebugUtilsMessengerEXT(impl->debug_utils_messenger);
#endif
        impl->instance.destroy();
    }

    void Context::recreate_swapchain(const uvec2& size) { recreate_swapchain(size, impl->surface_present_mode); }

    void Context::recreate_swapchain(const uvec2& size, const vk::PresentModeKHR present_mode)
    {
        impl->device.waitIdle();

        auto surface_capabilities = impl->physical_device.getSurfaceCapabilitiesKHR(impl->surface);

        impl->surface_extent = vk::Extent2D(
            std::clamp(size.x, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
            std::clamp(size.y, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height));

        vk::SwapchainCreateInfoKHR swapchain_create_info(
            {}, impl->surface, impl->present_image_count, impl->surface_format.format, impl->surface_format.colorSpace,
            impl->surface_extent, 1, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive, {}, surface_capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque, present_mode, true, impl->swapchain);

        impl->swapchain = impl->device.createSwapchainKHR(swapchain_create_info);

        // Destroy old swapchain
        vk::SwapchainKHR old_swapchain = swapchain_create_info.oldSwapchain;
        if (old_swapchain) impl->device.destroySwapchainKHR(old_swapchain);
        for (const auto& image_view : impl->swapchain_image_views) impl->device.destroyImageView(image_view);
        impl->swapchain_image_views.clear();

        // Create new swapchain
        impl->swapchain_images = impl->device.getSwapchainImagesKHR(impl->swapchain);
        impl->present_image_count = impl->swapchain_images.size();
        impl->surface_present_mode = present_mode;

        impl->swapchain_image_views.reserve(impl->present_image_count);
        for (const auto& image : impl->swapchain_images)
        {
            const vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            const vk::ImageViewCreateInfo image_view_create_info({}, image, vk::ImageViewType::e2D,
                                                                 impl->surface_format.format, {}, range);

            impl->swapchain_image_views.push_back(impl->device.createImageView(image_view_create_info));
        }
    }

    void Context::begin_frame() { impl->frame_provider.begin_frame(); }

    b8 Context::end_frame(const RendererImage& image, const vk::Extent3D& extent)
    {
        return impl->frame_provider.end_frame(image, extent);
    }

    void Context::submit_commands_immediate(std::function<void(CommandBuffer cmd)>&& function)
    {
        auto& cmd = impl->submit_command_buffer;

        cmd.begin();
        function(cmd);  // execute the function
        cmd.end();

        vk::SubmitInfo submit;
        submit.setCommandBuffers(cmd.get_handle());

        impl->graphics_queue.submit(submit, impl->upload_fence);
        VK_CHECK(impl->device.waitForFences(impl->upload_fence, true, MAG_TIMEOUT));
        impl->device.resetFences(impl->upload_fence);
        impl->device.resetCommandPool(impl->immediate_command_pool);
    }

    void Context::begin_timestamp()
    {
        if (!impl->is_query_supported) return;

        auto& command_buffer = get_curr_frame().command_buffer;

        command_buffer.get_handle().resetQueryPool(impl->query_pool, 0, impl->query_count);
        command_buffer.get_handle().writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, impl->query_pool, 0);
    }

    void Context::end_timestamp()
    {
        if (!impl->is_query_supported) return;

        auto& command_buffer = get_curr_frame().command_buffer;

        command_buffer.get_handle().writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, impl->query_pool, 1);
    }

    void Context::calculate_timestamp()
    {
        if (!impl->is_query_supported) return;

        impl->device.waitIdle();

        // We need to query using u64 and then convert to float
        struct T2
        {
                u64 begin = 0;
                u64 end = 0;
        };

        auto result = impl->device.getQueryPoolResult<T2>(impl->query_pool, 0, impl->query_count, sizeof(u64),
                                                          vk::QueryResultFlagBits::e64);

        if (result.result != vk::Result::eSuccess)
        {
            LOG_ERROR("Failed to get query pool result: {0}", vk::to_string(result.result));
            return;
        }

        // Convert to ms
        impl->timestamp.average = static_cast<f64>(result.value.end) * impl->timestamp_period * 1e-6 -
                                  static_cast<f64>(result.value.begin) * impl->timestamp_period * 1e-6;
    }

    const vk::Instance& Context::get_instance() const { return impl->instance; }
    const vk::Device& Context::get_device() const { return impl->device; }
    const vk::PhysicalDevice& Context::get_physical_device() const { return impl->physical_device; }
    const vk::Queue& Context::get_graphics_queue() const { return impl->graphics_queue; }
    const vk::SurfaceKHR& Context::get_surface() const { return impl->surface; }
    const vk::SurfaceFormatKHR& Context::get_surface_format() const { return impl->surface_format; }
    const vk::Extent2D& Context::get_surface_extent() const { return impl->surface_extent; }
    const vk::SwapchainKHR& Context::get_swapchain() const { return impl->swapchain; }
    const vk::Format& Context::get_swapchain_image_format() const { return impl->surface_format.format; }
    const vk::CommandPool& Context::get_command_pool() const { return impl->command_pool; }
    const vk::Fence& Context::get_upload_fence() const { return impl->upload_fence; }
    const std::vector<vk::Image>& Context::get_swapchain_images() const { return impl->swapchain_images; }
    const std::vector<vk::ImageView>& Context::get_swapchain_image_views() const { return impl->swapchain_image_views; }
    const VmaAllocator& Context::get_allocator() const { return impl->allocator; }
    const ProfileResult& Context::get_timestamp() const { return impl->timestamp; }
    const u32& Context::get_curr_frame_number() const { return impl->frame_provider.get_current_frame_number(); }

    const vk::PhysicalDeviceDescriptorBufferPropertiesEXT& Context::get_descriptor_buffer_properties() const
    {
        return impl->descriptor_buffer_properties;
    }

    vk::Format Context::get_supported_format(const std::vector<vk::Format>& candidates, const vk::ImageTiling tiling,
                                             const vk::FormatFeatureFlags features) const
    {
        for (const auto& format : candidates)
        {
            const vk::FormatProperties properties = impl->physical_device.getFormatProperties(format);

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

        return candidates[0];
    }

    vk::Format Context::get_supported_depth_format() const
    {
        return get_supported_format({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                    vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }

    Frame& Context::get_curr_frame() { return impl->frame_provider.get_current_frame(); }
    DescriptorLayoutCache& Context::get_descriptor_layout_cache() { return *impl->descriptor_layout_cache; }
    DescriptorAllocator& Context::get_descriptor_allocator() { return *impl->descriptor_allocator; }

    vk::SampleCountFlagBits Context::get_msaa_samples() const { return impl->msaa_samples; }
    u32 Context::get_queue_family_index() const { return impl->queue_family_index; }
    u32 Context::get_swapchain_image_index() const { return impl->frame_provider.get_swapchain_image_index(); }
    u32 Context::get_frame_count() const { return impl->frame_count; }
};  // namespace mag
