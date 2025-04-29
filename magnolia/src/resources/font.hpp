#pragma once

#include <map>

#include "core/types.hpp"
#include "math/types.hpp"
#include "resources/image.hpp"
#include "resources/resource.hpp"

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

    struct Font : public IResource
    {
            str name;
            std::map<c8, Character> characters;
    };

    namespace resource
    {
        ref<Font> get_font(const str& name);
    };
};  // namespace mag
