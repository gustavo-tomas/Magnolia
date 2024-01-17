#include "renderer/image.hpp"

#include "core/logger.hpp"

#define VMA_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

namespace mag
{
    void Image::initialize(const vk::Extent3D& extent, const vk::Format format, const vk::ImageUsageFlags image_usage,
                           const vk::ImageAspectFlags image_aspect, const u32 mip_levels,
                           const vk::SampleCountFlagBits msaa_samples)
    {
        auto& context = get_context();

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
