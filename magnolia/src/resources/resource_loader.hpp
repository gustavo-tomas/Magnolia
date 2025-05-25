#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Image;
    struct Material;
    struct Model;
    struct Font;
    struct Audio;
    struct ShaderResource;

    namespace resource
    {
        MAG_API b8 load(const str& file_path, Image* image);
        MAG_API b8 load(const str& file_path, Material* material);
        MAG_API b8 load(const str& file_path, Model* model);
        MAG_API b8 load(const str& file_path, Font* font);
        MAG_API b8 load(const str& file_path, Audio* audio);
        MAG_API b8 load(const str& file_path, ShaderResource* shader);

        b8 get_image_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels);
        b8 is_image_extension_supported(const str& extension_with_dot);
    };  // namespace resource
};      // namespace mag
