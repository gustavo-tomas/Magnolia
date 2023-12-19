#pragma once

#include <map>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "core/types.hpp"
#include "core/window.hpp"

#define VK_CHECK(result) ASSERT(result == vk::Result::eSuccess, "Vk check failed")

namespace mag
{
    static const std::map<vk::PhysicalDeviceType, str> physical_device_type_str = {
        {vk::PhysicalDeviceType::eOther, "Other"},
        {vk::PhysicalDeviceType::eIntegratedGpu, "Integrated Gpu"},
        {vk::PhysicalDeviceType::eDiscreteGpu, "Gpu"},
        {vk::PhysicalDeviceType::eVirtualGpu, "Virtual Gpu"},
        {vk::PhysicalDeviceType::eCpu, "Cpu"}};

    // This is not an exhaustive list
    static const str vendor_id_str(const u32 vendor_id)
    {
        const std::map<u32, str> vendors = {{0x1002, "Amd"}, {0x1010, "ImgTec"},   {0x10DE, "Nvidia"},
                                            {0x13B5, "Arm"}, {0x5143, "Qualcomm"}, {0x8086, "Intel"}};

        if (vendors.count(vendor_id)) return vendors.at(vendor_id);
        return "Unknown";
    }

    struct ContextCreateOptions
    {
            Window& window;
            str application_name = "Magnolia";
            str engine_name = "Magnolia";

            std::vector<const char*> instance_extensions;
            std::vector<const char*> device_extensions;
            std::vector<const char*> validation_layers;

            vk::PhysicalDeviceType preferred_device_type = vk::PhysicalDeviceType::eDiscreteGpu;
            u32 frame_count = 3;  // 3 for triple buffering
    };

    class Context
    {
        public:
            void initialize(const ContextCreateOptions& options);
            void shutdown();

            const vk::Instance& get_instance() const { return this->instance; };
            const vk::Device& get_device() const { return this->device; };
            const vk::PhysicalDevice& get_physical_device() const { return this->physical_device; };
            const vk::Queue& get_graphics_queue() const { return this->graphics_queue; };
            const vk::SurfaceKHR& get_surface() const { return this->surface; };
            const vk::SurfaceFormatKHR& get_surface_format() const { return this->surface_format; };

        private:
            vk::Instance instance;
            vk::SurfaceKHR surface;
            vk::SurfaceFormatKHR surface_format;
            vk::PhysicalDevice physical_device;
            vk::Device device;
            vk::PresentModeKHR surface_present_mode;
            vk::Queue graphics_queue;
            vk::DispatchLoaderDynamic dynamic_loader;
            vk::DebugUtilsMessengerEXT debug_utils_messenger;

            u32 api_version = {};
            u32 queue_family_index = {};
            u32 present_image_count = {};
    };
};  // namespace mag
