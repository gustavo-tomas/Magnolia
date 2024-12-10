#pragma once

#include "math/types.hpp"
#include "private/vulkan_fwd.hpp"

namespace mag
{
    vk::ClearValue mag_to_vk(const math::vec4& v);

    vk::Extent2D mag_to_vk(const math::vec2& v);
    vk::Extent3D mag_to_vk(const math::vec3& v);
    vk::Extent3D mag_to_vk(const math::uvec3& v);

    math::vec2 vk_to_mag(const vk::Extent2D& extent);
    math::vec3 vk_to_mag(const vk::Extent3D& extent);

    enum class Filter;
    enum class SamplerAddressMode;
    enum class SamplerMipmapMode;

    vk::Filter mag_to_vk(const Filter filter);
    vk::SamplerAddressMode mag_to_vk(const SamplerAddressMode address_mode);
    vk::SamplerMipmapMode mag_to_vk(const SamplerMipmapMode mip_map_mode);

    Filter vk_to_mag(const vk::Filter& filter);
    SamplerAddressMode vk_to_mag(const vk::SamplerAddressMode& address_mode);
    SamplerMipmapMode vk_to_mag(const vk::SamplerMipmapMode& mip_map_mode);

    enum class AttachmentState;

    vk::AttachmentLoadOp mag_to_vk(const AttachmentState state);

    enum class SampleCount;

    SampleCount vk_to_mag(const vk::SampleCountFlagBits sample_count);
    vk::SampleCountFlagBits mag_to_vk(const SampleCount sample_count);

    enum class AttachmentType;

    AttachmentType vk_to_mag(const vk::ImageAspectFlagBits image_aspect);
    vk::ImageAspectFlags mag_to_vk(const AttachmentType image_aspect);

    vk::PrimitiveTopology str_to_vk_topology(const str& topology);
    vk::PolygonMode str_to_vk_polygon_mode(const str& polygon_mode);
    vk::CullModeFlags str_to_vk_cull_mode(const str& cull_mode);
    vk::BlendOp str_to_vk_blend_op(const str& blend_op);
    vk::BlendFactor str_to_vk_blend_factor(const str& blend_factor);
};  // namespace mag
