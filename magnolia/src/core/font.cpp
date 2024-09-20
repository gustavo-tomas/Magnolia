#include "core/font.hpp"

#include "msdf_atlas_gen/msdfgen/msdfgen-ext.h"
#include "msdf_atlas_gen/msdfgen/msdfgen.h"
// #include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace mag
{
    // @TODO: transform this into a resource and load using the file system. use loadFontData to load a font from memory

    Font::Font(const std::filesystem::path &file_path)
    {
        if (msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype())
        {
            // msdfgen::loadFontData(FreetypeHandle * library, const byte *data, int length);

            const char *font_str = file_path.c_str();
            if (msdfgen::FontHandle *font = msdfgen::loadFont(ft, font_str))
            {
                msdfgen::Shape shape;
                if (msdfgen::loadGlyph(shape, font, 'C', msdfgen::FONT_SCALING_EM_NORMALIZED))
                {
                    shape.normalize();
                    //                      max. angle
                    msdfgen::edgeColoringSimple(shape, 3.0);
                    //          output width, height
                    msdfgen::Bitmap<float, 3> msdf(32, 32);
                    //                            scale, translation (in em's)
                    msdfgen::SDFTransformation t(msdfgen::Projection(32.0, msdfgen::Vector2(0.125, 0.125)),
                                                 msdfgen::Range(0.125));
                    msdfgen::generateMSDF(msdf, shape, t);
                    msdfgen::savePng(msdf, "output.png");
                }
                msdfgen::destroyFont(font);
            }
            msdfgen::deinitializeFreetype(ft);
        }
    }

    Font::~Font() {}
}  // namespace mag
