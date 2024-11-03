#include "renderer/renderer_image.hpp"

#include <vulkan/vulkan.hpp>

#include "math/vec.hpp"
#include "private/renderer_type_conversions.hpp"
#include "renderer/buffers.hpp"
#include "renderer/command.hpp"
#include "renderer/context.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    struct RendererImage::IMPL
    {
            IMPL(const uvec3& extent, const ImageType image_type, const vk::Format format,
                 const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect, const u32 mip_levels,
                 const SampleCount msaa_samples, const str& name)
                : name(name),
                  mip_levels(mip_levels),
                  format(format),
                  image_usage(image_usage),
                  image_aspect(image_aspect),
                  extent(extent),
                  type(image_type),
                  sampler(Filter::Linear, SamplerAddressMode::Repeat, SamplerMipmapMode::Linear, mip_levels),
                  msaa_samples(msaa_samples)
            {
            }

            ~IMPL() = default;

            str name;
            u32 mip_levels;

            vk::Format format;
            vk::Image image;
            vk::ImageView image_view;
            vk::ImageUsageFlags image_usage;
            vk::ImageAspectFlags image_aspect;

            uvec3 extent;
            ImageType type;
            Sampler sampler;
            SampleCount msaa_samples;

            VmaAllocation allocation;
    };

    RendererImage::RendererImage(const uvec3& extent, const ImageType image_type, const vk::Format format,
                                 const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect,
                                 const u32 mip_levels, const SampleCount msaa_samples, const str& name)
        : impl(new IMPL(extent, image_type, format, image_usage, image_aspect, mip_levels, msaa_samples, name))
    {
        create_image_and_view();
    }

    RendererImage::RendererImage(const uvec3& extent, const ImageType image_type, const std::vector<u8>& pixels,
                                 const vk::Format format, const vk::ImageUsageFlags image_usage,
                                 const vk::ImageAspectFlags image_aspect, const u32 mip_levels,
                                 const SampleCount msaa_samples, const str& name)
        : impl(new IMPL(extent, image_type, format, image_usage, image_aspect, mip_levels, msaa_samples, name))
    {
        create_image_and_view();

        set_pixels(pixels);
    }

    RendererImage::~RendererImage()
    {
        auto& context = get_context();

        context.get_device().destroyImageView(impl->image_view);
        vmaDestroyImage(context.get_allocator(), impl->image, impl->allocation);
    }

    void RendererImage::create_image_and_view()
    {
        auto& context = get_context();

        // Create image and image view
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = static_cast<VkFormat>(impl->format);
        image_create_info.extent = mag_to_vk(impl->extent);
        image_create_info.mipLevels = impl->mip_levels;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = static_cast<VkSampleCountFlagBits>(mag_to_vk(impl->msaa_samples));
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = static_cast<VkImageUsageFlags>(impl->image_usage);

        VmaAllocationCreateInfo vma_alloc_info = {};
        vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        vma_alloc_info.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vma_alloc_info.priority = 0.5f;

        if (impl->type == ImageType::Attachment)
        {
            vma_alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            vma_alloc_info.priority = 1.0f;
        }

        VK_CHECK(VK_CAST(vmaCreateImage(context.get_allocator(), &image_create_info, &vma_alloc_info,
                                        reinterpret_cast<VkImage*>(&impl->image), &impl->allocation, 0)));

        const vk::ImageSubresourceRange range(impl->image_aspect, 0, impl->mip_levels, 0, 1);
        const vk::ImageViewCreateInfo view_create_info({}, impl->image, vk::ImageViewType::e2D, impl->format, {},
                                                       range);

        impl->image_view = context.get_device().createImageView(view_create_info);
    }

    void RendererImage::set_pixels(const std::vector<u8>& pixels)
    {
        auto& context = get_context();

        const u64 texture_size = pixels.size();

        VulkanBuffer staging_buffer;
        staging_buffer.initialize(texture_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_AUTO,
                                  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        staging_buffer.copy(pixels.data(), texture_size);

        context.submit_commands_immediate(
            [&](CommandBuffer& cmd)
            {
                cmd.copy_buffer_to_image(staging_buffer, *this);

                cmd.transfer_layout(*this, vk::ImageLayout::eShaderReadOnlyOptimal,
                                    vk::ImageLayout::eTransferDstOptimal);

                vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1);
                vk::ImageMemoryBarrier barrier({}, {}, {}, {}, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                                               impl->image, range);

                i32 mip_width = impl->extent.x;
                i32 mip_height = impl->extent.y;

                // @TODO: use KTX to generate mip maps: https://www.khronos.org/ktx/
                for (u32 i = 1; i < impl->mip_levels; i++)
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
                    cmd.get_handle().blitImage(impl->image, vk::ImageLayout::eTransferSrcOptimal, impl->image,
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

                barrier.subresourceRange.baseMipLevel = impl->mip_levels - 1;
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

    const vk::Image& RendererImage::get_image() const { return impl->image; }
    const vk::ImageView& RendererImage::get_image_view() const { return impl->image_view; }
    const vk::Format& RendererImage::get_format() const { return impl->format; }
    const uvec3& RendererImage::get_extent() const { return impl->extent; }
    const Sampler& RendererImage::get_sampler() const { return impl->sampler; }
    const str& RendererImage::get_name() const { return impl->name; }
    u32 RendererImage::get_mip_levels() const { return impl->mip_levels; }
};  // namespace mag
