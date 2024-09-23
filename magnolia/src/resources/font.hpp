#pragma once

#include "core/types.hpp"
#include "resources/image.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "msdf_atlas_gen/msdf-atlas-gen/msdf-atlas-gen.h"
#pragma GCC diagnostic pop

// This implementation was based on the cherno's implementation: https://www.youtube.com/watch?v=iMuiim9loOg
// Also see: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/Font.h

namespace mag
{
#define PIXEL_RANGE 2.0  // @TODO: hardcoded?

    struct Font
    {
            Image atlas_image;

            msdf_atlas::FontGeometry font_geometry;
            std::vector<msdf_atlas::GlyphGeometry> glyphs;
    };

    class FontManager
    {
        public:
            ref<Font> get(const str& name);
            ref<Font> get_default();

        private:
            std::map<str, ref<Font>> fonts;
    };
}  // namespace mag
