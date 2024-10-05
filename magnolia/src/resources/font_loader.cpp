#include "resources/font_loader.hpp"

#include <vector>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "resources/font.hpp"

// @TODO: move this to file system
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "stb/stb_image_write.h"
#pragma GCC diagnostic pop

namespace mag
{
    // Edge coloring macros
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull

    template <typename T, typename S, i32 N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static void CreateAndCacheAtlas(const str& fontName, f32 fontSize,
                                    const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
                                    const msdf_atlas::FontGeometry& fontGeometry, const u32 width, const u32 height,
                                    Image& image)
    {
        (void)fontName;
        (void)fontSize;
        (void)fontGeometry;

        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width,
                                                                                                           height);
        generator.setAttributes(attributes);
        generator.setThreadCount(8);  // @TODO: hardcoded
        generator.generate(glyphs.data(), static_cast<i32>(glyphs.size()));

        msdfgen::BitmapConstRef<T, N> bitmap = generator.atlasStorage();

        const u64 image_size = bitmap.width * bitmap.height * N;

        image.width = bitmap.width;
        image.height = bitmap.height;
        image.channels = N;
        image.mip_levels = 1;
        image.format = ImageFormat::Unorm;
        image.pixels = std::vector<u8>(reinterpret_cast<const u8*>(bitmap.pixels),
                                       reinterpret_cast<const u8*>(bitmap.pixels) + image_size);

        // @TODO: move this to file system
        stbi_flip_vertically_on_write(true);
        if (!stbi_write_png((fontName + "_native.png").c_str(), image.width, image.height, image.channels,
                            image.pixels.data(), image.width * image.channels))
        {
            ASSERT(false, "@TODO: OOUUUFF");
        }
    }

    FontLoader::FontLoader() : freetype(msdfgen::initializeFreetype()) {}

    FontLoader::~FontLoader() { msdfgen::deinitializeFreetype(static_cast<msdfgen::FreetypeHandle*>(freetype)); }

    b8 FontLoader::load(const str& file_path, Font* font)
    {
        if (!font)
        {
            LOG_ERROR("Invalid font ptr");
            return false;
        }

        font->file_path = file_path;

        auto& app = get_application();
        auto& file_system = app.get_file_system();

        auto ft = static_cast<msdfgen::FreetypeHandle*>(freetype);
        if (!ft)
        {
            LOG_ERROR("Failed to initialize freetype");
            return false;
        }

        Buffer buffer;
        file_system.read_binary_data(file_path, buffer);

        msdfgen::FontHandle* font_handle = msdfgen::loadFontData(ft, buffer.data.data(), buffer.get_size());
        if (!font_handle)
        {
            LOG_ERROR("Failed to load font file: {0}", file_path);
            return false;
        }

        struct CharsetRange
        {
                u32 Begin, End;
        };

        // From imgui_draw.cpp
        static const CharsetRange charsetRanges[] = {{0x0020, 0x00FF}};

        msdf_atlas::Charset charset;
        for (CharsetRange range : charsetRanges)
        {
            for (u32 c = range.Begin; c <= range.End; c++) charset.add(c);
        }

        const f64 fontScale = 1.0;
        font->font_geometry = msdf_atlas::FontGeometry(&font->glyphs);
        i32 glyphsLoaded = font->font_geometry.loadCharset(font_handle, fontScale, charset);
        LOG_INFO("Loaded {0} glyphs from font (out of {1})", glyphsLoaded, charset.size());

        f64 emSize = 64.0;

        msdf_atlas::TightAtlasPacker atlasPacker;
        atlasPacker.setPixelRange(PIXEL_RANGE);
        atlasPacker.setMiterLimit(1.0);
        atlasPacker.setScale(emSize);

        const i32 remaining = atlasPacker.pack(font->glyphs.data(), static_cast<i32>(font->glyphs.size()));
        if (remaining != 0)
        {
            LOG_ERROR("Failed to pack font atlas from font: {0}", file_path);
            msdfgen::destroyFont(font_handle);

            return false;
        }

        i32 width, height;
        atlasPacker.getDimensions(width, height);
        emSize = atlasPacker.getScale();

        // Edge coloring
        u64 coloringSeed = 0;
        u64 glyphSeed = coloringSeed;

        for (msdf_atlas::GlyphGeometry& glyph : font->glyphs)
        {
            glyphSeed *= LCG_MULTIPLIER;
            glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
        }

        CreateAndCacheAtlas<u8, f32, 4, msdf_atlas::mtsdfGenerator>(
            file_path, static_cast<f32>(emSize), font->glyphs, font->font_geometry, width, height, font->atlas_image);

        msdfgen::destroyFont(font_handle);

        return true;
    }

    b8 FontLoader::is_extension_supported(const str& extension_with_dot) { return extension_with_dot == ".ttf"; }
};  // namespace mag
