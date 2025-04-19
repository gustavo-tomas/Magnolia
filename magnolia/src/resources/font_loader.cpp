// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/font.hpp"

//
#include <freetype2/freetype/freetype.h>
#include FT_FREETYPE_H
//

namespace mag
{
    namespace resource
    {
        b8 load(const str &file_path, Font *font)
        {
            if (!font)
            {
                LOG_ERROR("Invalid font ptr");
                return false;
            }

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load font file: {0}", file_path);
                return false;
            }

            FT_Library ft;
            if (FT_Init_FreeType(&ft))
            {
                LOG_ERROR("Failed to initialize FreeType Library");
                return false;
            }

            FT_Face face;
            if (FT_New_Memory_Face(ft, buffer.data.data(), buffer.get_size(), 0, &face))
            {
                LOG_ERROR("Failed to load font face: {0}", file_path);
                return false;
            }

            // @TODO: hardcoded pixel height
            FT_Set_Pixel_Sizes(face, 0, 48);

            // @TODO: hardcoded number of characters
            // Load first 128 characters of ASCII set
            for (u8 c = 0; c < 128; c++)
            {
                // Load character glyph
                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                {
                    LOG_ERROR("Failed to load glyph: {0}", c);
                    continue;
                }

                Character &character = font->characters[c];
                character.size = math::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
                character.bearing = math::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
                character.advance = math::ivec2(face->glyph->advance.x, face->glyph->advance.y);
                character.data = std::vector<u8>(
                    face->glyph->bitmap.buffer,
                    face->glyph->bitmap.buffer + (face->glyph->bitmap.width * face->glyph->bitmap.rows));
            }

            // Update font data
            font->name = file_path;

            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            return true;
        }
    };  // namespace resource
};      // namespace mag
