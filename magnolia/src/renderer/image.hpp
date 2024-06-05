#pragma once

#include <map>
#include <memory>

#include "assimp/material.h"
#include "renderer/context.hpp"
#include "renderer/sampler.hpp"

namespace mag
{
    enum class TextureType
    {
        Undefined = aiTextureType_UNKNOWN,
        Albedo = aiTextureType_DIFFUSE,
        Normal = aiTextureType_NORMALS
    };

    class Image
    {
        public:
            void initialize(const vk::Extent3D& extent, const vk::Format format, const vk::ImageUsageFlags image_usage,
                            const vk::ImageAspectFlags image_aspect, const u32 mip_levels = 1,
                            const vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1,
                            const TextureType type = TextureType::Undefined, const str& name = "");
            void shutdown();

            const vk::Image& get_image() const { return image; };
            const vk::ImageView& get_image_view() const { return image_view; };
            const vk::Format& get_format() const { return format; };
            const vk::Extent3D& get_extent() const { return extent; };
            const Sampler& get_sampler() const { return sampler; };
            const str& get_name() const { return name; };
            u32 get_mip_levels() const { return mip_levels; };
            TextureType get_type() const { return type; };

        private:
            Sampler sampler;
            vk::Format format;
            vk::Image image;
            vk::ImageView image_view;
            vk::Extent3D extent;
            VmaAllocation allocation;
            TextureType type;
            str name;
            u32 mip_levels;
    };

    class TextureManager
    {
        public:
            TextureManager() = default;
            ~TextureManager();

            std::shared_ptr<Image> load(const str& file, const TextureType type);

        private:
            std::map<str, std::shared_ptr<Image>> textures;
    };
};  // namespace mag
