#pragma once

#include <vulkan/vulkan_core.h>

#include "VkBootstrap.h"
#include "core/assert.hpp"
#include "core/types.hpp"
#include "gfx/backend/backend.hpp"
#include "math/types.hpp"
#include "vk_mem_alloc.h"

namespace mag
{
    namespace gfx
    {
        inline VkOffset2D mag_to_vk(const math::ivec2& offset)
        {
            const VkOffset2D vk_offset = {offset[0], offset[1]};
            return vk_offset;
        }

        inline math::ivec2 vk_to_mag(const VkOffset2D& offset)
        {
            const math::ivec2 mag_offset = {offset.x, offset.y};
            return mag_offset;
        }

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

        inline math::vec4 vk_to_mag(const VkClearColorValue& clear_color)
        {
            const math::vec4 mag_clear_color = {clear_color.float32[0], clear_color.float32[1], clear_color.float32[2],
                                                clear_color.float32[3]};
            return mag_clear_color;
        }

        inline VkClearColorValue mag_to_vk(const math::vec4& clear_color)
        {
            const VkClearColorValue vk_clear_color = {{clear_color[0], clear_color[1], clear_color[2], clear_color[3]}};
            return vk_clear_color;
        }

        inline VkShaderStageFlagBits mag_to_vk(const ShaderStage shader_stage)
        {
            switch (shader_stage)
            {
                case ShaderStage::Vertex:
                    return VK_SHADER_STAGE_VERTEX_BIT;
                    break;

                case ShaderStage::Fragment:
                    return VK_SHADER_STAGE_FRAGMENT_BIT;
                    break;
            }
        }

        inline ShaderStage vk_to_mag_shader_stage(const VkShaderStageFlags shader_stage)
        {
            switch (shader_stage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT:
                    return ShaderStage::Vertex;
                    break;

                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    return ShaderStage::Fragment;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled shader stage");
                    return ShaderStage::Vertex;
                    break;
            }
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

        inline vkb::QueueType mag_to_vk(const QueueType queue_type)
        {
            switch (queue_type)
            {
                case QueueType::Present:
                    return vkb::QueueType::present;
                    break;

                case QueueType::Graphics:
                    return vkb::QueueType::graphics;
                    break;

                case QueueType::Compute:
                    return vkb::QueueType::compute;
                    break;

                case QueueType::Transfer:
                    return vkb::QueueType::transfer;
                    break;
            }
        }

        inline QueueType vk_to_mag(const vkb::QueueType queue_type)
        {
            switch (queue_type)
            {
                case vkb::QueueType::present:
                    return QueueType::Present;
                    break;

                case vkb::QueueType::graphics:
                    return QueueType::Graphics;
                    break;

                case vkb::QueueType::compute:
                    return QueueType::Compute;
                    break;

                case vkb::QueueType::transfer:
                    return QueueType::Transfer;
                    break;
            }
        }

        inline VkCommandBufferLevel mag_to_vk(const CommandBufferLevel level)
        {
            switch (level)
            {
                case CommandBufferLevel::Primary:
                    return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    break;

                case CommandBufferLevel::Secondary:
                    return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                    break;
            }
        }

        inline CommandBufferLevel vk_to_mag(const VkCommandBufferLevel queue_type)
        {
            switch (queue_type)
            {
                case VK_COMMAND_BUFFER_LEVEL_PRIMARY:
                    return CommandBufferLevel::Primary;
                    break;

                case VK_COMMAND_BUFFER_LEVEL_SECONDARY:
                    return CommandBufferLevel::Secondary;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled command buffer level");
                    break;
            }
        }

        inline VkPrimitiveTopology mag_to_vk(const PrimitiveTopology primitive_topology)
        {
            switch (primitive_topology)
            {
                case PrimitiveTopology::TriangleList:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                    break;
            }
        }

        inline PrimitiveTopology vk_to_mag(const VkPrimitiveTopology primitive_topology)
        {
            switch (primitive_topology)
            {
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
                    return PrimitiveTopology::TriangleList;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled primitive topology list");
                    return PrimitiveTopology::TriangleList;
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

        inline VkImageLayout mag_to_vk(const TextureLayout image_layout)
        {
            switch (image_layout)
            {
                case TextureLayout::Undefined:
                    return VK_IMAGE_LAYOUT_UNDEFINED;
                    break;

                case TextureLayout::ColorAttachment:
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    break;

                case TextureLayout::Present:
                    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    break;

                case TextureLayout::TransferSrc:
                    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    break;

                case TextureLayout::TransferDst:
                    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    break;
            }
        }

        inline TextureLayout vk_to_mag(const VkImageLayout image_layout)
        {
            switch (image_layout)
            {
                case VK_IMAGE_LAYOUT_UNDEFINED:
                    return TextureLayout::Undefined;
                    break;

                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    return TextureLayout::ColorAttachment;
                    break;

                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                    return TextureLayout::Present;
                    break;

                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    return TextureLayout::TransferSrc;
                    break;

                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    return TextureLayout::TransferDst;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled texture layout");
                    break;
            }
        }

        inline TextureType vk_to_mag(const VkImageType image_type)
        {
            switch (image_type)
            {
                case VK_IMAGE_TYPE_1D:
                    return TextureType::Texture1D;
                    break;

                case VK_IMAGE_TYPE_2D:
                    return TextureType::Texture2D;
                    break;

                case VK_IMAGE_TYPE_3D:
                    return TextureType::Texture3D;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled texture type");
                    return TextureType::Texture2D;
                    break;
            }
        }

        inline VkImageType mag_to_vk(const TextureType image_type)
        {
            switch (image_type)
            {
                case TextureType::Texture1D:
                    return VK_IMAGE_TYPE_1D;
                    break;

                case TextureType::Texture2D:
                    return VK_IMAGE_TYPE_2D;
                    break;

                case TextureType::Texture3D:
                    return VK_IMAGE_TYPE_3D;
                    break;
            }
        }

        inline TextureViewType vk_to_mag(const VkImageViewType image_view_type)
        {
            switch (image_view_type)
            {
                case VK_IMAGE_VIEW_TYPE_1D:
                    return TextureViewType::Texture1D;
                    break;

                case VK_IMAGE_VIEW_TYPE_2D:
                    return TextureViewType::Texture2D;
                    break;

                case VK_IMAGE_VIEW_TYPE_3D:
                    return TextureViewType::Texture3D;
                    break;

                case VK_IMAGE_VIEW_TYPE_CUBE:
                    return TextureViewType::TextureCube;
                    break;

                case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
                    return TextureViewType::Texture1DArray;
                    break;

                case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
                    return TextureViewType::Texture2DArray;
                    break;

                case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
                    return TextureViewType::TextureCubeArray;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled texture view type");
                    return TextureViewType::Texture2D;
                    break;
            }
        }

        inline VkImageViewType mag_to_vk(const TextureViewType image_view_type)
        {
            switch (image_view_type)
            {
                case TextureViewType::Texture1D:
                    return VK_IMAGE_VIEW_TYPE_1D;
                    break;

                case TextureViewType::Texture2D:
                    return VK_IMAGE_VIEW_TYPE_2D;
                    break;

                case TextureViewType::Texture3D:
                    return VK_IMAGE_VIEW_TYPE_3D;
                    break;

                case TextureViewType::TextureCube:
                    return VK_IMAGE_VIEW_TYPE_CUBE;
                    break;

                case TextureViewType::Texture1DArray:
                    return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                    break;

                case TextureViewType::Texture2DArray:
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                    break;

                case TextureViewType::TextureCubeArray:
                    return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                    break;
            }
        }

        inline SampleCount vk_to_mag(const VkSampleCountFlagBits sample_count)
        {
            switch (sample_count)
            {
                case VK_SAMPLE_COUNT_1_BIT:
                    return SampleCount::e1;
                    break;

                case VK_SAMPLE_COUNT_2_BIT:
                    return SampleCount::e2;
                    break;

                case VK_SAMPLE_COUNT_4_BIT:
                    return SampleCount::e4;
                    break;

                case VK_SAMPLE_COUNT_8_BIT:
                    return SampleCount::e8;
                    break;

                case VK_SAMPLE_COUNT_16_BIT:
                    return SampleCount::e16;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled sample count");
                    return SampleCount::e1;
                    break;
            }
        }

        inline VkSampleCountFlagBits mag_to_vk(const SampleCount sample_count)
        {
            switch (sample_count)
            {
                case SampleCount::e1:
                    return VK_SAMPLE_COUNT_1_BIT;
                    break;

                case SampleCount::e2:
                    return VK_SAMPLE_COUNT_2_BIT;
                    break;

                case SampleCount::e4:
                    return VK_SAMPLE_COUNT_4_BIT;
                    break;

                case SampleCount::e8:
                    return VK_SAMPLE_COUNT_8_BIT;
                    break;

                case SampleCount::e16:
                    return VK_SAMPLE_COUNT_16_BIT;
                    break;
            }
        }

        inline VkImageAspectFlags mag_to_vk(const TextureAspect texture_aspect)
        {
            VkImageAspectFlags vk_aspect = {};

            if (IS_BIT_SET(texture_aspect, TextureAspect::None))
            {
                vk_aspect |= VK_IMAGE_ASPECT_NONE;
            }
            if (IS_BIT_SET(texture_aspect, TextureAspect::Color))
            {
                vk_aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if (IS_BIT_SET(texture_aspect, TextureAspect::Depth))
            {
                vk_aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (IS_BIT_SET(texture_aspect, TextureAspect::Stencil))
            {
                vk_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            return vk_aspect;
        }

        inline TextureAspect vk_to_mag_aspect(const VkImageAspectFlags texture_aspect)
        {
            TextureAspect mag_aspect = {};

            if (IS_BIT_SET(texture_aspect, VK_IMAGE_ASPECT_NONE))
            {
                mag_aspect |= TextureAspect::None;
            }
            if (IS_BIT_SET(texture_aspect, VK_IMAGE_ASPECT_COLOR_BIT))
            {
                mag_aspect |= TextureAspect::Color;
            }
            if (IS_BIT_SET(texture_aspect, VK_IMAGE_ASPECT_DEPTH_BIT))
            {
                mag_aspect |= TextureAspect::Depth;
            }
            if (IS_BIT_SET(texture_aspect, VK_IMAGE_ASPECT_STENCIL_BIT))
            {
                mag_aspect |= TextureAspect::Stencil;
            }

            return mag_aspect;
        }

        inline VkAccessFlags mag_to_vk(const AccessMask mask)
        {
            VkAccessFlags vk_access = {};

            if (IS_BIT_SET(mask, AccessMask::None))
            {
                vk_access |= VK_ACCESS_NONE;
            }
            if (IS_BIT_SET(mask, AccessMask::ColorAttachmentWrite))
            {
                vk_access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
            if (IS_BIT_SET(mask, AccessMask::TransferRead))
            {
                vk_access |= VK_ACCESS_TRANSFER_READ_BIT;
            }
            if (IS_BIT_SET(mask, AccessMask::TransferWrite))
            {
                vk_access |= VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            if (IS_BIT_SET(mask, AccessMask::MemoryRead))
            {
                vk_access |= VK_ACCESS_MEMORY_READ_BIT;
            }
            if (IS_BIT_SET(mask, AccessMask::MemoryWrite))
            {
                vk_access |= VK_ACCESS_MEMORY_WRITE_BIT;
            }

            return vk_access;
        }

        inline AccessMask vk_to_mag_acess(const VkAccessFlags mask)
        {
            AccessMask mag_access = {};

            if (IS_BIT_SET(mask, VK_ACCESS_NONE))
            {
                mag_access |= AccessMask::None;
            }
            if (IS_BIT_SET(mask, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT))
            {
                mag_access |= AccessMask::ColorAttachmentWrite;
            }
            if (IS_BIT_SET(mask, VK_ACCESS_TRANSFER_READ_BIT))
            {
                mag_access |= AccessMask::TransferRead;
            }
            if (IS_BIT_SET(mask, VK_ACCESS_TRANSFER_WRITE_BIT))
            {
                mag_access |= AccessMask::TransferWrite;
            }
            if (IS_BIT_SET(mask, VK_ACCESS_MEMORY_READ_BIT))
            {
                mag_access |= AccessMask::MemoryRead;
            }
            if (IS_BIT_SET(mask, VK_ACCESS_MEMORY_WRITE_BIT))
            {
                mag_access |= AccessMask::MemoryWrite;
            }

            return mag_access;
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

            if (IS_BIT_SET(usage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT))
            {
                mag_usage |= TextureUsage::TransferSrc;
            }
            if (IS_BIT_SET(usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT))
            {
                mag_usage |= TextureUsage::TransferDst;
            }
            if (IS_BIT_SET(usage, VK_IMAGE_USAGE_SAMPLED_BIT))
            {
                mag_usage |= TextureUsage::Sampled;
            }
            if (IS_BIT_SET(usage, VK_IMAGE_USAGE_STORAGE_BIT))
            {
                mag_usage |= TextureUsage::Storage;
            }
            if (IS_BIT_SET(usage, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
            {
                mag_usage |= TextureUsage::ColorAttachment;
            }
            if (IS_BIT_SET(usage, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
            {
                mag_usage |= TextureUsage::DepthStencilAttachment;
            }

            return mag_usage;
        }

        inline VkPipelineStageFlags mag_to_vk(const PipelineStage stage)
        {
            VkPipelineStageFlags vk_stage = {};

            if (IS_BIT_SET(stage, PipelineStage::TopOfPipe))
            {
                vk_stage |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }
            if (IS_BIT_SET(stage, PipelineStage::ColorAttachmentOutput))
            {
                vk_stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            }
            if (IS_BIT_SET(stage, PipelineStage::BottomOfPipe))
            {
                vk_stage |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }
            if (IS_BIT_SET(stage, PipelineStage::Transfer))
            {
                vk_stage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            if (IS_BIT_SET(stage, PipelineStage::AllCommands))
            {
                vk_stage |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            }

            return vk_stage;
        }

        inline PipelineStage vk_to_mag_stage(const VkPipelineStageFlags stage)
        {
            PipelineStage mag_stage = {};

            if (IS_BIT_SET(stage, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT))
            {
                mag_stage |= PipelineStage::TopOfPipe;
            }
            if (IS_BIT_SET(stage, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT))
            {
                mag_stage |= PipelineStage::ColorAttachmentOutput;
            }
            if (IS_BIT_SET(stage, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT))
            {
                mag_stage |= PipelineStage::BottomOfPipe;
            }
            if (IS_BIT_SET(stage, VK_PIPELINE_STAGE_TRANSFER_BIT))
            {
                mag_stage |= PipelineStage::Transfer;
            }
            if (IS_BIT_SET(stage, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT))
            {
                mag_stage |= PipelineStage::AllCommands;
            }

            return mag_stage;
        }

        inline VkBufferUsageFlags mag_to_vk(const BufferUsage usage)
        {
            VkBufferUsageFlags vk_usage = {};

            if (IS_BIT_SET(usage, BufferUsage::Vertex))
            {
                vk_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            if (IS_BIT_SET(usage, BufferUsage::Index))
            {
                vk_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            if (IS_BIT_SET(usage, BufferUsage::Uniform))
            {
                vk_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            if (IS_BIT_SET(usage, BufferUsage::Storage))
            {
                vk_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            if (IS_BIT_SET(usage, BufferUsage::TransferSrc))
            {
                vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            if (IS_BIT_SET(usage, BufferUsage::TransferDst))
            {
                vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }

            return vk_usage;
        }

        inline BufferUsage vk_to_mag_buffer_usage(const VkBufferUsageFlags usage)
        {
            BufferUsage mag_usage = {};

            if (IS_BIT_SET(usage, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
            {
                mag_usage |= BufferUsage::Vertex;
            }
            if (IS_BIT_SET(usage, VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
            {
                mag_usage |= BufferUsage::Index;
            }
            if (IS_BIT_SET(usage, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
            {
                mag_usage |= BufferUsage::Uniform;
            }
            if (IS_BIT_SET(usage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
            {
                mag_usage |= BufferUsage::Storage;
            }
            if (IS_BIT_SET(usage, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
            {
                mag_usage |= BufferUsage::TransferSrc;
            }
            if (IS_BIT_SET(usage, VK_BUFFER_USAGE_TRANSFER_DST_BIT))
            {
                mag_usage |= BufferUsage::TransferDst;
            }

            return mag_usage;
        }

        inline VmaMemoryUsage mag_to_vk(const MemoryUsage usage)
        {
            switch (usage)
            {
                case MemoryUsage::Auto:
                    return VMA_MEMORY_USAGE_AUTO;
                    break;

                case MemoryUsage::PreferHost:
                    return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
                    break;

                case MemoryUsage::PreferDevice:
                    return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                    break;
            }
        }

        inline MemoryUsage mag_to_vk(const VmaMemoryUsage usage)
        {
            switch (usage)
            {
                case VMA_MEMORY_USAGE_AUTO:
                    return MemoryUsage::Auto;
                    break;

                case VMA_MEMORY_USAGE_AUTO_PREFER_HOST:
                    return MemoryUsage::PreferHost;
                    break;

                case VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE:
                    return MemoryUsage::PreferDevice;
                    break;

                default:
                    MAG_ASSERT(false, "Unhandled memory usage");
                    break;
            }
        }
    };  // namespace gfx
};      // namespace mag
