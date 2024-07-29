#pragma once

#include "core/logger.hpp"
#include "core/types.hpp"
#include "vulkan/vulkan.hpp"

namespace mag
{
    inline vk::PrimitiveTopology str_to_vk_topology(const str& topology)
    {
        if (topology == "Triangle") return vk::PrimitiveTopology::eTriangleList;
        if (topology == "Line") return vk::PrimitiveTopology::eLineList;

        ASSERT(false, "Topology '" + topology + "' not supported");
        return vk::PrimitiveTopology::eTriangleList;
    }

    inline vk::PolygonMode str_to_vk_polygon_mode(const str& polygon_mode)
    {
        if (polygon_mode == "Fill") return vk::PolygonMode::eFill;
        if (polygon_mode == "Line") return vk::PolygonMode::eLine;

        ASSERT(false, "Polygon mode '" + polygon_mode + "' not supported");
        return vk::PolygonMode::eFill;
    }

    inline vk::CullModeFlags const str_to_vk_cull_mode(const str& cull_mode)
    {
        if (cull_mode == "None") return vk::CullModeFlagBits::eNone;
        if (cull_mode == "Back") return vk::CullModeFlagBits::eBack;

        ASSERT(false, "Cull mode '" + cull_mode + "' not supported");
        return vk::CullModeFlagBits::eNone;
    }

    inline vk::BlendOp str_to_vk_blend_op(const str& blend_op)
    {
        if (blend_op == "Add") return vk::BlendOp::eAdd;

        ASSERT(false, "Blend Operation '" + blend_op + "' not supported");
        return vk::BlendOp::eAdd;
    }

    inline vk::BlendFactor str_to_vk_blend_factor(const str& blend_factor)
    {
        if (blend_factor == "One") return vk::BlendFactor::eOne;
        if (blend_factor == "OneMinusSrcAlpha") return vk::BlendFactor::eOneMinusSrcAlpha;
        if (blend_factor == "SrcAlpha") return vk::BlendFactor::eSrcAlpha;

        ASSERT(false, "Blend Operation '" + blend_factor + "' not supported");
        return vk::BlendFactor::eOne;
    }
};  // namespace mag
