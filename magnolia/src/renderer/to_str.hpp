#pragma once

#include <map>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"

namespace mag
{
    namespace str_mappings
    {
        static const std::map<vk::PhysicalDeviceType, str> physical_device_type_mapping = {
            {vk::PhysicalDeviceType::eOther, "Other"},
            {vk::PhysicalDeviceType::eIntegratedGpu, "Integrated Gpu"},
            {vk::PhysicalDeviceType::eDiscreteGpu, "Gpu"},
            {vk::PhysicalDeviceType::eVirtualGpu, "Virtual Gpu"},
            {vk::PhysicalDeviceType::eCpu, "Cpu"}};

        static const std::map<vk::PresentModeKHR, str> present_mode_mapping = {
            {vk::PresentModeKHR::eImmediate, "Immediate"},
            {vk::PresentModeKHR::eMailbox, "Mailbox"},
            {vk::PresentModeKHR::eFifo, "Fifo"},
            {vk::PresentModeKHR::eFifoRelaxed, "Fifo Relaxed"},
            {vk::PresentModeKHR::eSharedDemandRefresh, "Shared Demand Refresh"},
            {vk::PresentModeKHR::eSharedContinuousRefresh, "Shared Continuous Refresh"}};

        static const std::map<u32, str> vendor_mapping = {{0x1002, "Amd"}, {0x1010, "ImgTec"},   {0x10DE, "Nvidia"},
                                                          {0x13B5, "Arm"}, {0x5143, "Qualcomm"}, {0x8086, "Intel"}};
    };  // namespace str_mappings

    inline str to_str(const vk::PhysicalDeviceType physical_device_type)
    {
        return str_mappings::physical_device_type_mapping.at(physical_device_type);
    }

    inline str to_str(const vk::PresentModeKHR present_mode)
    {
        return str_mappings::present_mode_mapping.at(present_mode);
    }

    inline str to_str(const u32 vendor_id)
    {
        return str_mappings::vendor_mapping.at(vendor_id);
        if (str_mappings::vendor_mapping.count(vendor_id)) return str_mappings::vendor_mapping.at(vendor_id);
        return "Unknown";
    }
};  // namespace mag
