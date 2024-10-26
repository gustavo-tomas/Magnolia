#include "private/renderer_type_conversions.hpp"

#include <vulkan/vulkan.hpp>

#include "core/logger.hpp"
#include "math/vec.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    vk::ClearValue const mag_to_vk(const math::vec4& v)
    {
        const vk::ClearValue vk_clear_value({v.r, v.g, v.b, v.a});

        return vk_clear_value;
    }

    vk::Extent2D const mag_to_vk(const math::vec2& v)
    {
        const vk::Extent2D vk_extent(v.r, v.g);

        return vk_extent;
    }

    vk::Extent3D const mag_to_vk(const math::vec3& v)
    {
        const vk::Extent3D vk_extent(v.r, v.g, v.b);

        return vk_extent;
    }

    math::vec2 const vk_to_mag(const vk::Extent2D& extent)
    {
        const math::vec2 v(extent.width, extent.height);

        return v;
    }

    math::vec3 const vk_to_mag(const vk::Extent3D& extent)
    {
        const math::vec3 v(extent.width, extent.height, extent.depth);

        return v;
    }

    vk::Filter mag_to_vk(const Filter filter)
    {
        switch (filter)
        {
            case Filter::Linear:
                return vk::Filter::eLinear;
                break;

            case Filter::Nearest:
                return vk::Filter::eNearest;
                break;

            default:
                LOG_ERROR("Invalid filter value");
                return vk::Filter::eLinear;
                break;
        }
    }

    vk::SamplerAddressMode mag_to_vk(const SamplerAddressMode address_mode)
    {
        switch (address_mode)
        {
            case SamplerAddressMode::Repeat:
                return vk::SamplerAddressMode::eRepeat;
                break;

            case SamplerAddressMode::MirroredRepeat:
                return vk::SamplerAddressMode::eMirroredRepeat;
                break;

            case SamplerAddressMode::ClampToEdge:
                return vk::SamplerAddressMode::eClampToEdge;
                break;

            case SamplerAddressMode::ClampToBorder:
                return vk::SamplerAddressMode::eClampToBorder;
                break;

            case SamplerAddressMode::MirrorClampToEdge:
                return vk::SamplerAddressMode::eMirrorClampToEdge;
                break;

            default:
                LOG_ERROR("Invalid address mode value");
                return vk::SamplerAddressMode::eRepeat;
                break;
        }
    }

    vk::SamplerMipmapMode mag_to_vk(const SamplerMipmapMode mip_map_mode)
    {
        switch (mip_map_mode)
        {
            case SamplerMipmapMode::Linear:
                return vk::SamplerMipmapMode::eLinear;
                break;

            case SamplerMipmapMode::Nearest:
                return vk::SamplerMipmapMode::eNearest;
                break;

            default:
                LOG_ERROR("Invalid mip map mode value");
                return vk::SamplerMipmapMode::eLinear;
                break;
        }
    }

    Filter vk_to_mag(const vk::Filter& filter)
    {
        switch (filter)
        {
            case vk::Filter::eLinear:
                return Filter::Linear;
                break;

            case vk::Filter::eNearest:
                return Filter::Nearest;
                break;

            default:
                LOG_ERROR("Invalid filter value");
                return Filter::Linear;
                break;
        }
    }

    SamplerAddressMode vk_to_mag(const vk::SamplerAddressMode& address_mode)
    {
        switch (address_mode)
        {
            case vk::SamplerAddressMode::eRepeat:
                return SamplerAddressMode::Repeat;
                break;

            case vk::SamplerAddressMode::eMirroredRepeat:
                return SamplerAddressMode::MirroredRepeat;
                break;

            case vk::SamplerAddressMode::eClampToEdge:
                return SamplerAddressMode::ClampToEdge;
                break;

            case vk::SamplerAddressMode::eClampToBorder:
                return SamplerAddressMode::ClampToBorder;
                break;

            case vk::SamplerAddressMode::eMirrorClampToEdge:
                return SamplerAddressMode::MirrorClampToEdge;
                break;

            default:
                LOG_ERROR("Invalid address mode value");
                return SamplerAddressMode::Repeat;
                break;
        }
    }

    SamplerMipmapMode vk_to_mag(const vk::SamplerMipmapMode& mip_map_mode)
    {
        switch (mip_map_mode)
        {
            case vk::SamplerMipmapMode::eLinear:
                return SamplerMipmapMode::Linear;
                break;

            case vk::SamplerMipmapMode::eNearest:
                return SamplerMipmapMode::Nearest;
                break;

            default:
                LOG_ERROR("Invalid mip map mode value");
                return SamplerMipmapMode::Linear;
                break;
        }
    }
};  // namespace mag
