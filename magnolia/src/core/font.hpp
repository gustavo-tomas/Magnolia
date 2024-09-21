#pragma once

#include <filesystem>

#include "core/types.hpp"
#include "renderer/renderer_image.hpp"
#include "resources/image.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "msdf_atlas_gen/msdf-atlas-gen/msdf-atlas-gen.h"
#pragma GCC diagnostic pop

// This implementation was based on the cherno's implementation: https://www.youtube.com/watch?v=iMuiim9loOg

// @TODO: pls cleanup before merging ty x0x0

namespace mag
{
    struct InternalFontData
    {
            msdf_atlas::FontGeometry FontGeometry;
            std::vector<msdf_atlas::GlyphGeometry> Glyphs;
    };

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