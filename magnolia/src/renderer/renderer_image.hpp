#pragma once

#include "renderer/context.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    class RendererImage
    {
        public:
            RendererImage(const vk::Extent3D& extent, const vk::Format format, const vk::ImageUsageFlags image_usage,
                          const vk::ImageAspectFlags image_aspect, const u32 mip_levels = 1,
                          const vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1,
                          const str& name = "");

            RendererImage(const vk::Extent3D& extent, const std::vector<u8>& pixels, const vk::Format format,
                          const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect,
                          const u32 mip_levels = 1,
                          const vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1,
                          const str& name = "");

            ~RendererImage();

            // The dimensions, mip levels, channels, etc are not changed, only the image pixels
            void set_pixels(const std::vector<u8>& pixels);

            const vk::Image& get_image() const { return image; };
            const vk::ImageView& get_image_view() const { return image_view; };
            const vk::Format& get_format() const { return format; };
            const vk::Extent3D& get_extent() const { return extent; };
            const Sampler& get_sampler() const { return sampler; };
            const str& get_name() const { return name; };
            u32 get_mip_levels() const { return mip_levels; };

        private:
            void create_image_and_view();

            str name;
            u32 mip_levels;
            Sampler sampler;

            vk::Format format;
            vk::Image image;
            vk::ImageView image_view;
            vk::Extent3D extent;
            vk::SampleCountFlagBits msaa_samples;
            vk::ImageUsageFlags image_usage;
            vk::ImageAspectFlags image_aspect;

            VmaAllocation allocation;
    };
};  // namespace mag
