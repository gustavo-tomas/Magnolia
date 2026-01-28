#include "magnolia/resources/font.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"

//
#include "freetype/freetype.h"
#include FT_FREETYPE_H
//

namespace mag
{
    namespace resource
    {
        FontLoader::FontLoader() = default;

        FontLoader::~FontLoader() = default;

        IResource *FontLoader::load(const str &file_path)
        {
            FontResource *font = new FontResource();

            if (font == nullptr)
            {
                LOG_ERROR("Invalid font ptr");
                delete font;
                return nullptr;
            }

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load font file: {0}", file_path);
                delete font;
                return nullptr;
            }

            FT_Library ft = nullptr;
            if (FT_Init_FreeType(&ft) != 0)
            {
                LOG_ERROR("Failed to initialize FreeType Library");
                delete font;
                return nullptr;
            }

            FT_Face face = nullptr;
            if (FT_New_Memory_Face(ft, buffer.data.data(), static_cast<i64>(buffer.get_size()), 0, &face) != 0)
            {
                LOG_ERROR("Failed to load font face: {0}", file_path);
                delete font;
                return nullptr;
            }

            // @TODO: hardcoded pixel height
            FT_Set_Pixel_Sizes(face, 0, 48);

            // @TODO: hardcoded number of characters
            // Load first 128 characters of ASCII set
            for (u8 c = 0; c < 128; c++)
            {
                // Load character glyph
                if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
                {
                    LOG_ERROR("Failed to load glyph: {0}", c);
                    continue;
                }

                Character &character = font->characters[static_cast<c8>(c)];
                character.size = math::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
                character.bearing = math::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
                character.advance = math::ivec2(face->glyph->advance.x, face->glyph->advance.y);

                // Some glyphs have no bitmpa data (i.e. space)
                if (face->glyph->bitmap.width > 0 && face->glyph->bitmap.rows > 0 &&
                    (face->glyph->bitmap.buffer != nullptr))
                {
                    character.data =
                        std::vector<u8>(face->glyph->bitmap.buffer,
                                        face->glyph->bitmap.buffer +
                                            (static_cast<u64>(face->glyph->bitmap.width * face->glyph->bitmap.rows)));

                    character.texture.channels = 1;
                    character.texture.width = character.size.x;
                    character.texture.height = character.size.y;
                    character.texture.pixels = character.data;
                }

                // Empty data for whitespace or non-visual characters
                else
                {
                    character.data.clear();
                }
            }

            // Update font data
            font->name = file_path;
            font->file_path = file_path;

            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            return font;
        }
    };  // namespace resource
};  // namespace mag
