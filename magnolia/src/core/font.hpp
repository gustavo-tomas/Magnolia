#pragma once

#include <filesystem>

#include "core/types.hpp"
#include "renderer/renderer_image.hpp"
#include "resources/image.hpp"

// This implementation was based on the cherno's implementation: https://www.youtube.com/watch?v=iMuiim9loOg

// @TODO: pls cleanup before merging ty x0x0

namespace mag
{
    struct InternalFontData;

    class Font
    {
        public:
            Font(const std::filesystem::path& file_path);
            ~Font();

            InternalFontData* internal_data = nullptr;
            Image* atlas_image = nullptr;
            ref<RendererImage> m_AtlasTexture;
    };

}  // namespace mag