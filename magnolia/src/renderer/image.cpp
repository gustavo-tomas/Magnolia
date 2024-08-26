#include "renderer/image.hpp"

#include "core/application.hpp"

namespace mag
{
#define DEFAULT_TEXTURE_NAME "magnolia/assets/images/DefaultAlbedoSeamless.png"

    std::shared_ptr<Image> TextureManager::get(const str& name)
    {
        // Texture found
        auto it = textures.find(name);
        if (it != textures.end())
        {
            return it->second;
        }

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& image_loader = app.get_image_loader();

        // Else load image from disk and create a new texture
        Image* image = image_loader.load(name);

        // Send image data to the GPU
        renderer.add_image(image);

        if (image == nullptr)
        {
            LOG_ERROR("Texture '{0}' not found, using default", name);

            image = image_loader.load(DEFAULT_TEXTURE_NAME);
            ASSERT(image, "Default texture has not been loaded");
        }

        textures[name] = std::shared_ptr<Image>(image);
        return textures[name];
    }

    std::shared_ptr<Image> TextureManager::get_default() { return get(DEFAULT_TEXTURE_NAME); }

    RendererImage::RendererImage(const vk::Extent3D& extent, const vk::Format format,
                                 const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect,
                                 const u32 mip_levels, const vk::SampleCountFlagBits msaa_samples, const str& name)
        : name(name),
          mip_levels(mip_levels),
          sampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear, mip_levels),
          format(format),
          extent(extent),
          msaa_samples(msaa_samples),
          image_usage(image_usage),
          image_aspect(image_aspect)
    {
        create_image_and_view();
    }

    RendererImage::RendererImage(const vk::Extent3D& extent, const std::vector<u8>& pixels, const vk::Format format,
                                 const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect,
                                 const u32 mip_levels, const vk::SampleCountFlagBits msaa_samples, const str& name)
        : name(name),
          mip_levels(mip_levels),
          sampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear, mip_levels),
          format(format),
          extent(extent),
          msaa_samples(msaa_samples),
          image_usage(image_usage),
          image_aspect(image_aspect)
    {
        create_image_and_view();

        auto& context = get_context();

        const u64 texture_size = pixels.size();

        Buffer staging_buffer;
        staging_buffer.initialize(texture_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        staging_buffer.copy(pixels.data(), texture_size);

        context.submit_commands_immediate(
            [&](CommandBuffer cmd)
            {
                cmd.copy_buffer_to_image(staging_buffer, *this);

                cmd.transfer_layout(*this, vk::ImageLayout::eShaderReadOnlyOptimal,
                                    vk::ImageLayout::eTransferDstOptimal);

                vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1);
                vk::ImageMemoryBarrier barrier({}, {}, {}, {}, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
                                               range);

                i32 mip_width = extent.width;
                i32 mip_height = extent.height;

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
                    cmd.get_handle().blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image,
                                               vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

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
    }

    void RendererImage::create_image_and_view()
    {
        auto& context = get_context();

        // Create image and image view
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

        VK_CHECK(VK_CAST(vmaCreateImage(context.get_allocator(), &image_create_info, &vma_alloc_info,
                                        reinterpret_cast<VkImage*>(&this->image), &allocation, 0)));

        const vk::ImageSubresourceRange range(image_aspect, 0, mip_levels, 0, 1);
        const vk::ImageViewCreateInfo view_create_info({}, image, vk::ImageViewType::e2D, format, {}, range);

        this->image_view = context.get_device().createImageView(view_create_info);
    }

    RendererImage::~RendererImage()
    {
        auto& context = get_context();

        context.get_device().destroyImageView(this->image_view);
        vmaDestroyImage(context.get_allocator(), image, allocation);
    }
};  // namespace mag
