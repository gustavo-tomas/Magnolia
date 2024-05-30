#pragma once

#include "core/types.hpp"
#include "vulkan/vulkan.hpp"

namespace mag
{
    inline vk::ClearValue const vec_to_vk_clear_value(const math::vec4& v)
    {
        const vk::ClearValue vk_clear_value({v.r, v.g, v.b, v.a});

        return vk_clear_value;
    }

    inline vk::Extent3D const vec_to_vk_extent(const math::vec3& v)
    {
        const vk::Extent3D vk_extent(v.r, v.g, v.b);

        return vk_extent;
    }

    inline math::vec2 const vk_extent_to_vec(const vk::Extent2D& extent)
    {
        const math::vec2 v(extent.width, extent.height);

        return v;
    }
};  // namespace mag
