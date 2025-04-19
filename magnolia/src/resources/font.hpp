#pragma once

#include <map>

#include "core/types.hpp"
#include "math/types.hpp"
#include "resources/image.hpp"

namespace mag
{
    struct Character
    {
            Image texture;         // @TODO: temporary - texture with glyph bitmap data
            math::ivec2 size;      // Size of glyph
            math::ivec2 bearing;   // Offset from baseline to left/top of glyph
            math::uvec2 advance;   // Offset to advance to next glyph
            std::vector<u8> data;  // Char data
    };

    struct Font
    {
            str name;
            std::map<c8, Character> characters;
    };

    class FontManager
    {
        public:
            FontManager();

            ref<Font> get(const str& name);

        private:
            std::map<str, ref<Font>> fonts;
    };
};  // namespace mag
