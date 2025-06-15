#pragma once

#include <map>

#include "core/types.hpp"
#include "math/types.hpp"
#include "resources/resource.hpp"
#include "resources/texture.hpp"

namespace mag
{
    struct Character
    {
            TextureResource texture;  // @TODO: temporary - texture with glyph bitmap data
            math::ivec2 size;         // Size of glyph
            math::ivec2 bearing;      // Offset from baseline to left/top of glyph
            math::uvec2 advance;      // Offset to advance to next glyph
            std::vector<u8> data;     // Char data
    };

    struct FontResource : public IResource
    {
            std::map<c8, Character> characters;
    };

    namespace resource
    {
        class FontLoader : public IResourceLoader
        {
            public:
                FontLoader();
                ~FontLoader();

                virtual IResource* load(const str& file_path) override;
        };
    };  // namespace resource
};      // namespace mag
