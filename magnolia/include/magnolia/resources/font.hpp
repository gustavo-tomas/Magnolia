#pragma once

#include <map>

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/resources/resource.hpp"
#include "magnolia/resources/texture.hpp"

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
            std::unordered_map<c8, Character> characters;
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
