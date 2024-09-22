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
// Also see: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/Font.h

// @TODO: pls cleanup before merging ty x0x0

namespace mag
{

#define PIXEL_RANGE 2.0  // @TODO: hardcoded?

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

            // @TODO: temp
            static ref<Font> get_default()
            {
                static ref<Font> default_font = nullptr;

                if (!default_font)
                {
                    default_font = create_ref<Font>("/usr/share/fonts/truetype/open-sans/OpenSans-Regular.ttf");
                }

                return default_font;
            }
    };

}  // namespace mag