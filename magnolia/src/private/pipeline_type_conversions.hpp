#pragma once

#include <vulkan/vulkan_handles.hpp>

#include "core/types.hpp"

namespace mag
{
    vk::PrimitiveTopology str_to_vk_topology(const str& topology);
    vk::PolygonMode str_to_vk_polygon_mode(const str& polygon_mode);
    vk::CullModeFlags const str_to_vk_cull_mode(const str& cull_mode);
    vk::BlendOp str_to_vk_blend_op(const str& blend_op);
    vk::BlendFactor str_to_vk_blend_factor(const str& blend_factor);
};  // namespace mag
