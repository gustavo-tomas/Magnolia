#pragma once

#include <map>
#include <memory>

#include "renderer/context.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    struct Image
    {
            u8 channels;
            u32 width;
            u32 height;
            u32 mip_levels;
            std::vector<u8> pixels;
    };

    class RendererImage
    {
        public:
            RendererImage(const vk::Extent3D& extent, const vk::Format format, const vk::ImageUsageFlags image_usage,
                          const vk::ImageAspectFlags image_aspect, const u32 mip_levels = 1,
                          const vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1,
                          const str& name = "");

            RendererImage(const vk::Extent3D& extent, const u8 channels, const void* pixels, const vk::Format format,
                          const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect,
                          const u32 mip_levels = 1,
                          const vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1,
                          const str& name = "");

            ~RendererImage();

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

    class TextureManager
    {
        public:
            std::shared_ptr<Image> get(const str& name);
            std::shared_ptr<Image> get_default();

        private:
            std::map<str, std::shared_ptr<Image>> textures;
    };
};  // namespace mag
