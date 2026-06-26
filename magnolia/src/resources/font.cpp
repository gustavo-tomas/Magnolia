#include "magnolia/resources/font.hpp"

#include "freetype/freetype.h"
#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"

namespace mag
{
    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, FontResource* resource)
        {
            (void)rm;

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load font file: {0}", file_path);
                return false;
            }

            FT_Library ft = nullptr;
            if (FT_Init_FreeType(&ft) != 0)
            {
                LOG_ERROR("Failed to initialize FreeType Library");
                return false;
            }

            FT_Face face = nullptr;
            if (FT_New_Memory_Face(ft, buffer.data.data(), static_cast<i64>(buffer.get_size()), 0, &face) != 0)
            {
                LOG_ERROR("Failed to load font face: {0}", file_path);
                return false;
            }

            // @TODO: hardcoded pixel height and number of characters
            const u32 pixel_width = 0;
            const u32 pixel_height = 48;
            const u8 num_of_characters = 128;

            FT_Set_Pixel_Sizes(face, pixel_width, pixel_height);

            // Load first N characters of ASCII set
            for (u8 c = 0; c < num_of_characters; c++)
            {
                // Load character glyph
                if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
                {
                    LOG_ERROR("Failed to load glyph: {0}", c);
                    continue;
                }

                Character& character = resource->characters[static_cast<c8>(c)];
                character.size = math::uvec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
                character.bearing = math::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
                character.advance = math::uvec2(face->glyph->advance.x, face->glyph->advance.y);

                // Some glyphs have no bitmpa data (i.e. space)
                if (face->glyph->bitmap.width > 0 && face->glyph->bitmap.rows > 0 &&
                    (face->glyph->bitmap.buffer != nullptr))
                {
                    const u64 bitmap_size = 1ULL * face->glyph->bitmap.width * face->glyph->bitmap.rows;
                    const std::span pixels_span(face->glyph->bitmap.buffer, bitmap_size);

                    character.data = std::vector<u8>(pixels_span.begin(), pixels_span.end());
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
            resource->name = file_path;
            resource->file_path = file_path;

            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            return true;
        }
    };  // namespace resource
};  // namespace mag
