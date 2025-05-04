#pragma once

#include <vulkan/vulkan_core.h>

#include "core/assert.hpp"
#include "core/types.hpp"
#include "gfx/gfx.hpp"
#include "math/types.hpp"

namespace mag
{
    namespace gfx
    {
        inline VkExtent3D mag_to_vk(const math::uvec3& extent)
        {
            const VkExtent3D vk_extent = {extent[0], extent[1], extent[2]};
            return vk_extent;
        }

        inline VkExtent2D mag_to_vk(const math::uvec2& extent)
        {
            const VkExtent2D vk_extent = {extent[0], extent[1]};
            return vk_extent;
        }

        inline math::uvec3 vk_to_mag(const VkExtent3D& extent)
        {
            const math::uvec3 mag_extent = {extent.width, extent.height, extent.depth};
            return mag_extent;
        }

        inline math::uvec2 vk_to_mag(const VkExtent2D& extent)
        {
            const math::uvec2 mag_extent = {extent.width, extent.height};
            return mag_extent;
        }

        inline VkPresentModeKHR mag_to_vk(const PresentMode present_mode)
        {
            switch (present_mode)
            {
                case PresentMode::Mailbox:
                    return VK_PRESENT_MODE_MAILBOX_KHR;
                    break;

                case PresentMode::Immediate:
                    return VK_PRESENT_MODE_IMMEDIATE_KHR;
                    break;

                case PresentMode::Fifo:
                    return VK_PRESENT_MODE_FIFO_KHR;
                    break;
            }
        }

        inline PresentMode vk_to_mag(const VkPresentModeKHR present_mode)
        {
            switch (present_mode)
            {
                case VK_PRESENT_MODE_MAILBOX_KHR:
                    return PresentMode::Mailbox;
                    break;

                case VK_PRESENT_MODE_IMMEDIATE_KHR:
                    return PresentMode::Immediate;
                    break;

                case VK_PRESENT_MODE_FIFO_KHR:
                    return PresentMode::Fifo;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled present mode");
                    return PresentMode::Mailbox;
                    break;
            }
        }

        inline VkFormat mag_to_vk(const Format format)
        {
            switch (format)
            {
                case Format::R8G8B8A8_UNORM:
                    return VK_FORMAT_R8G8B8A8_UNORM;
                    break;

                case Format::B8G8R8A8_UNORM:
                    return VK_FORMAT_B8G8R8A8_UNORM;
                    break;

                case Format::B8G8R8A8_SRGB:
                    return VK_FORMAT_B8G8R8A8_SRGB;
                    break;

                case Format::R16G16B16A16_SFLOAT:
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
                    break;

                case Format::R32G32B32A32_SFLOAT:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                    break;

                case Format::D32_SFLOAT:
                    return VK_FORMAT_D32_SFLOAT;
                    break;

                case Format::D24_UNORM_S8_UINT:
                    return VK_FORMAT_D24_UNORM_S8_UINT;
                    break;
            }
        }

        inline Format vk_to_mag(const VkFormat format)
        {
            switch (format)
            {
                case VK_FORMAT_R8G8B8A8_UNORM:
                    return Format::R8G8B8A8_UNORM;
                    break;

                case VK_FORMAT_B8G8R8A8_UNORM:
                    return Format::B8G8R8A8_UNORM;
                    break;

                case VK_FORMAT_B8G8R8A8_SRGB:
                    return Format::B8G8R8A8_SRGB;
                    break;

                case VK_FORMAT_R16G16B16A16_SFLOAT:
                    return Format::R16G16B16A16_SFLOAT;
                    break;

                case VK_FORMAT_R32G32B32A32_SFLOAT:
                    return Format::R32G32B32A32_SFLOAT;
                    break;

                case VK_FORMAT_D32_SFLOAT:
                    return Format::D32_SFLOAT;
                    break;

                case VK_FORMAT_D24_UNORM_S8_UINT:
                    return Format::D24_UNORM_S8_UINT;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled format");
                    return Format::R8G8B8A8_UNORM;
            }
        }

        inline VkImageUsageFlags mag_to_vk(const TextureUsage usage)
        {
            VkImageUsageFlags vk_usage = {};

            if (IS_BIT_SET(usage, TextureUsage::TransferSrc))
            {
                vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }
            if (IS_BIT_SET(usage, TextureUsage::TransferDst))
            {
                vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            if (IS_BIT_SET(usage, TextureUsage::Sampled))
            {
                vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            }
            if (IS_BIT_SET(usage, TextureUsage::Storage))
            {
                vk_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
            }
            if (IS_BIT_SET(usage, TextureUsage::ColorAttachment))
            {
                vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            if (IS_BIT_SET(usage, TextureUsage::DepthStencilAttachment))
            {
                vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }

            return vk_usage;
        }

        inline TextureUsage vk_to_mag(const VkImageUsageFlags usage)
        {
            TextureUsage mag_usage = {};

            if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            {
                mag_usage |= TextureUsage::TransferSrc;
            }
            if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            {
                mag_usage |= TextureUsage::TransferDst;
            }
            if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
            {
                mag_usage |= TextureUsage::Sampled;
            }
            if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            {
                mag_usage |= TextureUsage::Storage;
            }
            if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            {
                mag_usage |= TextureUsage::ColorAttachment;
            }
            if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                mag_usage |= TextureUsage::DepthStencilAttachment;
            }

            return mag_usage;
        }
    };  // namespace gfx
};      // namespace mag
