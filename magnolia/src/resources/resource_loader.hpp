#pragma once

#include "core/types.hpp"

namespace mag
{
    struct TextureResource;
    struct MaterialResource;
    struct ModelResource;
    struct FontResource;
    struct AudioResource;
    struct ShaderResource;

    namespace resource
    {
        MAG_API b8 load(const str& file_path, TextureResource* texture);
        MAG_API b8 load(const str& file_path, MaterialResource* material);
        MAG_API b8 load(const str& file_path, ModelResource* model);
        MAG_API b8 load(const str& file_path, FontResource* font);
        MAG_API b8 load(const str& file_path, AudioResource* audio);
        MAG_API b8 load(const str& file_path, ShaderResource* shader);

        b8 get_image_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels);
        b8 is_image_extension_supported(const str& extension_with_dot);
    };  // namespace resource
};      // namespace mag
