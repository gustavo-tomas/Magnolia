#pragma once

#include <map>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"

namespace mag
{
    namespace str_mappings
    {
        static const std::map<u32, str> vendor_mapping = {{0x1002, "Amd"}, {0x1010, "ImgTec"},   {0x10DE, "Nvidia"},
                                                          {0x13B5, "Arm"}, {0x5143, "Qualcomm"}, {0x8086, "Intel"}};
    };  // namespace str_mappings

    inline str to_str(const u32 vendor_id)
    {
        if (str_mappings::vendor_mapping.count(vendor_id)) return str_mappings::vendor_mapping.at(vendor_id);
        return "Unknown";
    }
};  // namespace mag
