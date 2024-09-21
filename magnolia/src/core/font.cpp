#include "core/font.hpp"

#include <vector>

#include "core/application.hpp"
#include "core/assert.hpp"
#include "renderer/renderer_image.hpp"
#include "resources/image.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "msdf_atlas_gen/msdf-atlas-gen/msdf-atlas-gen.h"
#pragma GCC diagnostic pop

#include "msdf_atlas_gen/msdfgen/msdfgen-ext.h"
#include "msdf_atlas_gen/msdfgen/msdfgen.h"

namespace mag
{
    // @TODO: transform this into a resource and load using the file system. use loadFontData to load a font from memory

    struct InternalFontData
    {
            msdf_atlas::FontGeometry FontGeometry;
            std::vector<msdf_atlas::GlyphGeometry> Glyphs;
    };

    template <typename T, typename S, i32 N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static ref<RendererImage> CreateAndCacheAtlas(const str& fontName, f32 fontSize,
                                                  const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
                                                  const msdf_atlas::FontGeometry& fontGeometry, uint32_t width,
                                                  uint32_t height, Image* image)
    {
        (void)fontName;
        (void)fontSize;
        (void)fontGeometry;

        auto& app = get_application();
        auto& renderer = app.get_renderer();

        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width,
                                                                                                           height);
        generator.setAttributes(attributes);
        generator.setThreadCount(8);  // @TODO: hardcoded
        generator.generate(glyphs.data(), (i32)glyphs.size());

        msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

        const u64 channels = 4;  // @TODO: check if error here
        const u64 image_size = bitmap.width * bitmap.height * channels;
        const u64 true_image_size = bitmap.width * bitmap.height * channels;

        image = new Image();
        image->width = bitmap.width;
        image->height = bitmap.height;
        image->channels = channels;
        image->mip_levels = 1;
        image->pixels.resize(image_size);
        image->pixels.insert(image->pixels.begin(), (u8*)bitmap.pixels, (u8*)bitmap.pixels + true_image_size);

        return renderer.upload_image(image);
    }

    Font::Font(const std::filesystem::path& file_path) : internal_data(new InternalFontData())
    {
        msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
        ASSERT(ft, "Failed to initialize free type");

        const char* fileString = file_path.c_str();

        msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString);
        if (!font)
        {
            LOG_ERROR("Failed to load font: {0}", fileString);
            return;
        }

        struct CharsetRange
        {
                uint32_t Begin, End;
        };

        // From imgui_draw.cpp
        static const CharsetRange charsetRanges[] = {{0x0020, 0x00FF}};

        msdf_atlas::Charset charset;
        for (CharsetRange range : charsetRanges)
        {
            for (uint32_t c = range.Begin; c <= range.End; c++) charset.add(c);
        }

        f64 fontScale = 1.0;
        internal_data->FontGeometry = msdf_atlas::FontGeometry(&internal_data->Glyphs);
        i32 glyphsLoaded = internal_data->FontGeometry.loadCharset(font, fontScale, charset);
        LOG_INFO("Loaded {0} glyphs from font (out of {1})", glyphsLoaded, charset.size());

        f64 emSize = 40.0;

        msdf_atlas::TightAtlasPacker atlasPacker;
        atlasPacker.setPixelRange(2.0);
        atlasPacker.setMiterLimit(1.0);
        atlasPacker.setScale(emSize);

        i32 remaining = atlasPacker.pack(internal_data->Glyphs.data(), (i32)internal_data->Glyphs.size());
        ASSERT(remaining == 0, "Failed to pack font atlas");

        i32 width, height;
        atlasPacker.getDimensions(width, height);
        emSize = atlasPacker.getScale();

        // Edge coloring
        if (true)
        {
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8
            // if MSDF || MTSDF

            u64 coloringSeed = 0;
            b8 expensiveColoring = false;
            if (expensiveColoring)
            {
                msdf_atlas::Workload(
                    [&glyphs = internal_data->Glyphs, &coloringSeed](i32 i, i32 threadNo) -> b8
                    {
                        (void)threadNo;
                        u64 glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
                        glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                        return true;
                    },
                    internal_data->Glyphs.size())
                    .finish(THREAD_COUNT);
            }
            else
            {
                u64 glyphSeed = coloringSeed;
                for (msdf_atlas::GlyphGeometry& glyph : internal_data->Glyphs)
                {
                    glyphSeed *= LCG_MULTIPLIER;
                    glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                }
            }
        }

        m_AtlasTexture = CreateAndCacheAtlas<u8, f32, 4, msdf_atlas::mtsdfGenerator>(
            "Test", (f32)emSize, internal_data->Glyphs, internal_data->FontGeometry, width, height, atlas_image);

        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
    }

    Font::~Font() { delete internal_data; }
}  // namespace mag
