#include "private/renderer_type_conversions.hpp"

#include <vulkan/vulkan.hpp>

#include "core/logger.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    vk::ClearValue mag_to_vk(const math::vec4& v)
    {
        const vk::ClearValue vk_clear_value({v.r, v.g, v.b, v.a});

        return vk_clear_value;
    }

    vk::Extent2D mag_to_vk(const math::vec2& v)
    {
        const vk::Extent2D vk_extent(v.r, v.g);

        return vk_extent;
    }

    vk::Extent3D mag_to_vk(const math::vec3& v)
    {
        const vk::Extent3D vk_extent(v.r, v.g, v.b);

        return vk_extent;
    }

    vk::Extent3D mag_to_vk(const math::uvec3& v) { return mag_to_vk(vec3(v)); }

    math::vec2 vk_to_mag(const vk::Extent2D& extent)
    {
        const math::vec2 v(extent.width, extent.height);

        return v;
    }

    math::vec3 vk_to_mag(const vk::Extent3D& extent)
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
                LOG_ERROR("Invalid Filter");
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
                LOG_ERROR("Invalid SamplerAddressMode");
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
                LOG_ERROR("Invalid Filter");
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
                LOG_ERROR("Invalid Filter");
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
                LOG_ERROR("Invalid SamplerAddressMode");
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
                LOG_ERROR("Invalid SamplerMipmapMode");
                return SamplerMipmapMode::Linear;
                break;
        }
    }

    vk::AttachmentLoadOp mag_to_vk(const AttachmentState state)
    {
        switch (state)
        {
            case AttachmentState::Clear:
                return vk::AttachmentLoadOp::eClear;
                break;

            case AttachmentState::Load:
                return vk::AttachmentLoadOp::eLoad;
                break;

            default:
                LOG_ERROR("Invalid AttachmentState");
                return vk::AttachmentLoadOp::eClear;
                break;
        }
    }

    SampleCount vk_to_mag(const vk::SampleCountFlagBits sample_count)
    {
        switch (sample_count)
        {
            case vk::SampleCountFlagBits::e1:
                return SampleCount::_1;
                break;

            case vk::SampleCountFlagBits::e2:
                return SampleCount::_2;
                break;

            case vk::SampleCountFlagBits::e4:
                return SampleCount::_4;
                break;

            case vk::SampleCountFlagBits::e8:
                return SampleCount::_8;
                break;

            case vk::SampleCountFlagBits::e16:
                return SampleCount::_16;
                break;

            case vk::SampleCountFlagBits::e32:
                return SampleCount::_32;
                break;

            case vk::SampleCountFlagBits::e64:
                return SampleCount::_64;
                break;

            default:
                LOG_ERROR("Invalid SampleCount");
                return SampleCount::_1;
                break;
        }
    }

    vk::SampleCountFlagBits mag_to_vk(const SampleCount sample_count)
    {
        switch (sample_count)
        {
            case SampleCount::_1:
                return vk::SampleCountFlagBits::e1;
                break;

            case SampleCount::_2:
                return vk::SampleCountFlagBits::e2;
                break;

            case SampleCount::_4:
                return vk::SampleCountFlagBits::e4;
                break;

            case SampleCount::_8:
                return vk::SampleCountFlagBits::e8;
                break;

            case SampleCount::_16:
                return vk::SampleCountFlagBits::e16;
                break;

            case SampleCount::_32:
                return vk::SampleCountFlagBits::e32;
                break;

            case SampleCount::_64:
                return vk::SampleCountFlagBits::e64;
                break;

            default:
                LOG_ERROR("Invalid SampleCount");
                return vk::SampleCountFlagBits::e1;
                break;
        }
    }

    vk::PrimitiveTopology str_to_vk_topology(const str& topology)
    {
        if (topology == "Triangle") return vk::PrimitiveTopology::eTriangleList;
        if (topology == "Line") return vk::PrimitiveTopology::eLineList;

        LOG_ERROR("Invalid topology '{0}'", topology);
        return vk::PrimitiveTopology::eTriangleList;
    }

    vk::PolygonMode str_to_vk_polygon_mode(const str& polygon_mode)
    {
        if (polygon_mode == "Fill") return vk::PolygonMode::eFill;
        if (polygon_mode == "Line") return vk::PolygonMode::eLine;

        LOG_ERROR("Invalid polygon mode '{0}'", polygon_mode);
        return vk::PolygonMode::eFill;
    }

    vk::CullModeFlags str_to_vk_cull_mode(const str& cull_mode)
    {
        if (cull_mode == "None") return vk::CullModeFlagBits::eNone;
        if (cull_mode == "Back") return vk::CullModeFlagBits::eBack;

        LOG_ERROR("Invalid cull mode '{0}'", cull_mode);
        return vk::CullModeFlagBits::eNone;
    }

    vk::BlendOp str_to_vk_blend_op(const str& blend_op)
    {
        if (blend_op == "Add") return vk::BlendOp::eAdd;

        LOG_ERROR("Invalid blend operation '{0}'", blend_op);
        return vk::BlendOp::eAdd;
    }

    vk::BlendFactor str_to_vk_blend_factor(const str& blend_factor)
    {
        if (blend_factor == "One") return vk::BlendFactor::eOne;
        if (blend_factor == "OneMinusSrcAlpha") return vk::BlendFactor::eOneMinusSrcAlpha;
        if (blend_factor == "SrcAlpha") return vk::BlendFactor::eSrcAlpha;

        LOG_ERROR("Invalid blend operation '{0}'", blend_factor);
        return vk::BlendFactor::eOne;
    }
};  // namespace mag
