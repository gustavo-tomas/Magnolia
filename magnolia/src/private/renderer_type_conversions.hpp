#pragma once

#include "core/math.hpp"

namespace vk
{
    union ClearValue;
    struct Extent2D;
    struct Extent3D;
};  // namespace vk

namespace mag
{
    vk::ClearValue const vec_to_vk_clear_value(const math::vec4& v);

    vk::Extent2D const vec_to_vk_extent(const math::vec2& v);
    vk::Extent3D const vec_to_vk_extent(const math::vec3& v);

    math::vec2 const vk_extent_to_vec(const vk::Extent2D& extent);
    math::vec3 const vk_extent_to_vec(const vk::Extent3D& extent);
};  // namespace mag
