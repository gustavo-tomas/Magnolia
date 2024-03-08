#pragma once

#include "renderer/context.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    Image* load_image(const str& file);

    class Image
    {
        public:
            void initialize(const vk::Extent3D& extent, const vk::Format format, const vk::ImageUsageFlags image_usage,
                            const vk::ImageAspectFlags image_aspect, const u32 mip_levels = 1,
                            const vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1);
            void shutdown();

            const vk::Image& get_image() const { return image; };
            const vk::ImageView& get_image_view() const { return image_view; };
            const vk::Format& get_format() const { return format; };
            const vk::Extent3D& get_extent() const { return extent; };
            const Sampler& get_sampler() const { return sampler; };
            u32 get_mip_levels() const { return mip_levels; };

        private:
            Sampler sampler;
            vk::Format format;
            vk::Image image;
            vk::ImageView image_view;
            vk::Extent3D extent;
            VmaAllocation allocation;

            u32 mip_levels;
    };
};  // namespace mag
