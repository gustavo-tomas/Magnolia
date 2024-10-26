#pragma once

#include "math/types.hpp"

namespace vk
{
    union ClearValue;
    struct Extent2D;
    struct Extent3D;

    enum class Filter;
    enum class SamplerAddressMode;
    enum class SamplerMipmapMode;
};  // namespace vk

namespace mag
{
    vk::ClearValue const mag_to_vk(const math::vec4& v);

    vk::Extent2D const mag_to_vk(const math::vec2& v);
    vk::Extent3D const mag_to_vk(const math::vec3& v);

    math::vec2 const vk_to_mag(const vk::Extent2D& extent);
    math::vec3 const vk_to_mag(const vk::Extent3D& extent);

    enum class Filter;
    enum class SamplerAddressMode;
    enum class SamplerMipmapMode;

    vk::Filter mag_to_vk(const Filter filter);
    vk::SamplerAddressMode mag_to_vk(const SamplerAddressMode address_mode);
    vk::SamplerMipmapMode mag_to_vk(const SamplerMipmapMode mip_map_mode);

    Filter vk_to_mag(const vk::Filter& filter);
    SamplerAddressMode vk_to_mag(const vk::SamplerAddressMode& address_mode);
    SamplerMipmapMode vk_to_mag(const vk::SamplerMipmapMode& mip_map_mode);
};  // namespace mag
