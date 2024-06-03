#include "renderer/image.hpp"

#include "core/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mag
{
    TextureManager::~TextureManager()
    {
        for (const auto& texture_pair : textures)
        {
            const auto& texture = texture_pair.second;
            texture->shutdown();
        }
    }

    std::shared_ptr<Image> TextureManager::load(const str& file, const TextureType type)
    {
        auto it = textures.find(file);
        if (it != textures.end()) return it->second;

        auto& context = get_context();

        i32 tex_width, tex_height, tex_channels;

        stbi_uc* pixels = stbi_load(file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if (pixels == NULL)
        {
            LOG_ERROR("Failed to load texture file: {0}", file);
            stbi_image_free(pixels);

            // Return the default texture
            it = textures.find("magnolia/assets/images/DefaultAlbedoSeamless.png");
            if (it != textures.end())
            {
                return it->second;
            }

            // If the default texture has not been loaded, there is a serious logic error and we should not proceed
            ASSERT(false, "Default albedo texture has not been loaded");
        }

        const u64 image_size = tex_width * tex_height * 4;  // @TODO: hardcoded
        vk::Format image_format = vk::Format::eR8G8B8A8Srgb;

        Buffer staging_buffer;
        staging_buffer.initialize(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        staging_buffer.copy(reinterpret_cast<void*>(pixels), image_size);

        stbi_image_free(pixels);

        const vk::Extent3D image_extent(static_cast<u32>(tex_width), static_cast<u32>(tex_height), 1);
        const u32 mip_levels =
            static_cast<u32>(std::floor(std::log2(std::max(image_extent.width, image_extent.height)))) + 1;

        Image* image = new Image();
        image->initialize(image_extent, image_format,
                          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                              vk::ImageUsageFlagBits::eTransferDst,
                          vk::ImageAspectFlagBits::eColor, mip_levels, vk::SampleCountFlagBits::e1, type);

        context.submit_commands_immediate(
            [&](CommandBuffer cmd)
            {
                cmd.copy_buffer_to_image(staging_buffer, *image);

                cmd.transfer_layout(*image, vk::ImageLayout::eShaderReadOnlyOptimal,
                                    vk::ImageLayout::eTransferDstOptimal);

                vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1);
                vk::ImageMemoryBarrier barrier({}, {}, {}, {}, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                                               image->get_image(), range);

                i32 mip_width = tex_width;
                i32 mip_height = tex_height;

                // @TODO: improve layout transition
                for (u32 i = 1; i < mip_levels; i++)
                {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                    barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                    cmd.get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                     vk::PipelineStageFlagBits::eTransfer,
                                                     vk::DependencyFlagBits::eByRegion, {}, {}, barrier);

                    vk::ImageSubresourceLayers src_subresource(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
                    vk::ImageSubresourceLayers dst_subresource(vk::ImageAspectFlagBits::eColor, i, 0, 1);

                    std::array<vk::Offset3D, 2> src_offsets = {};
                    src_offsets.at(1).setX(mip_width).setY(mip_height).setZ(1);

                    std::array<vk::Offset3D, 2> dst_offsets = {};
                    dst_offsets.at(1)
                        .setX(mip_width > 1 ? mip_width / 2 : 1)
                        .setY(mip_height > 1 ? mip_height / 2 : 1)
                        .setZ(1);

                    vk::ImageBlit blit(src_subresource, src_offsets, dst_subresource, dst_offsets);
                    cmd.get_handle().blitImage(image->get_image(), vk::ImageLayout::eTransferSrcOptimal,
                                               image->get_image(), vk::ImageLayout::eTransferDstOptimal, blit,
                                               vk::Filter::eLinear);

                    barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                    cmd.get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                     vk::PipelineStageFlagBits::eFragmentShader,
                                                     vk::DependencyFlagBits::eByRegion, {}, {}, barrier);

                    if (mip_width > 1) mip_width /= 2;
                    if (mip_height > 1) mip_height /= 2;
                }

                barrier.subresourceRange.baseMipLevel = mip_levels - 1;
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                cmd.get_handle().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                 vk::PipelineStageFlagBits::eFragmentShader,
                                                 vk::DependencyFlagBits::eByRegion, {}, {}, barrier);
            });

        staging_buffer.shutdown();

        textures[file] = std::shared_ptr<Image>(image);
        return textures[file];
    }

    void Image::initialize(const vk::Extent3D& extent, const vk::Format format, const vk::ImageUsageFlags image_usage,
                           const vk::ImageAspectFlags image_aspect, const u32 mip_levels,
                           const vk::SampleCountFlagBits msaa_samples, const TextureType type)
    {
        auto& context = get_context();

        this->type = type;
        this->format = format;
        this->extent = extent;
        this->mip_levels = mip_levels;
        this->sampler.initialize(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear,
                                 this->mip_levels);

        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = static_cast<VkFormat>(format);
        image_create_info.extent = extent;
        image_create_info.mipLevels = mip_levels;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = static_cast<VkSampleCountFlagBits>(msaa_samples);
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = static_cast<VkImageUsageFlags>(image_usage);

        VmaAllocationCreateInfo vma_alloc_info = {};
        vma_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vma_alloc_info.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkImage vk_image;
        VK_CHECK(VK_CAST(
            vmaCreateImage(context.get_allocator(), &image_create_info, &vma_alloc_info, &vk_image, &allocation, 0)));

        this->image = vk::Image(vk_image);

        const vk::ImageSubresourceRange range(image_aspect, 0, mip_levels, 0, 1);
        const vk::ImageViewCreateInfo view_create_info({}, image, vk::ImageViewType::e2D, format, {}, range);

        this->image_view = context.get_device().createImageView(view_create_info);
    }

    void Image::shutdown()
    {
        auto& context = get_context();

        this->sampler.shutdown();
        context.get_device().destroyImageView(this->image_view);
        vmaDestroyImage(context.get_allocator(), image, allocation);
    }

};  // namespace mag
