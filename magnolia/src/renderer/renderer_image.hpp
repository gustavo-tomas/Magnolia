#pragma once

#include <vector>

#include "math/types.hpp"
#include "private/vulkan_fwd.hpp"

namespace mag
{
    using namespace mag::math;

    enum class SampleCount
    {
        _1,
        _2,
        _4,
        _8,
        _16,
        _32,
        _64
    };

    enum class ImageType
    {
        Texture,
        Attachment
    };

    class Sampler;

    class RendererImage
    {
        public:
            RendererImage(const uvec3& extent, const ImageType image_type, const vk::Format format,
                          const vk::ImageUsageFlags image_usage, const vk::ImageAspectFlags image_aspect,
                          const u32 mip_levels = 1, const SampleCount msaa_samples = SampleCount::_1,
                          const str& name = "");

            RendererImage(const uvec3& extent, const ImageType image_type, const std::vector<u8>& pixels,
                          const vk::Format format, const vk::ImageUsageFlags image_usage,
                          const vk::ImageAspectFlags image_aspect, const u32 mip_levels = 1,
                          const SampleCount msaa_samples = SampleCount::_1, const str& name = "");

            ~RendererImage();

            // The dimensions, mip levels, channels, etc are not changed, only the image pixels
            void set_pixels(const std::vector<u8>& pixels);

            // @TODO: ugly fix
            // Indicates if the corresponding descriptor sets should be updated.
            b8 dirty = true;

            const vk::Image& get_image() const;
            const vk::ImageView& get_image_view() const;
            const vk::Format& get_format() const;
            const uvec3& get_extent() const;
            const Sampler& get_sampler() const;
            const str& get_name() const;
            u32 get_mip_levels() const;

        private:
            void create_image_and_view();

            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
