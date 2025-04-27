#include "resources/font.hpp"

#include "renderer/renderer.hpp"
#include "resources/resource_loader.hpp"

// @TODO: async loading

namespace mag
{
    FontManager::FontManager() {}

    ref<Font> FontManager::get(const str& name)
    {
        auto it = fonts.find(name);
        if (it != fonts.end())
        {
            return it->second;
        }

        // Create a new font
        Font* font = new Font();
        fonts[name] = ref<Font>(font);

        // Upload character texture to the GPU
        if (resource::load(name, font))
        {
            for (auto& [c, info] : font->characters)
            {
                Image& texture = info.texture;

                texture.channels = 1;
                texture.width = info.size.x;
                texture.height = info.size.y;
                texture.pixels = info.data;

                if (texture.width > 0 && texture.height > 0 && texture.pixels.size() == texture.width * texture.height)
                {
                    gfx::upload_image(&texture, SamplerAddressMode::ClampToEdge);
                }
            }
        }

        return fonts[name];
    }
};  // namespace mag
