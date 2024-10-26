#pragma once

#include "math/types.hpp"

namespace vk
{
    union ClearValue;
    struct Extent2D;
    struct Extent3D;
};  // namespace vk

namespace mag
{
    vk::ClearValue const mag_to_vk(const math::vec4& v);

    vk::Extent2D const mag_to_vk(const math::vec2& v);
    vk::Extent3D const mag_to_vk(const math::vec3& v);

    math::vec2 const vk_to_mag(const vk::Extent2D& extent);
    math::vec3 const vk_to_mag(const vk::Extent3D& extent);
};  // namespace mag
