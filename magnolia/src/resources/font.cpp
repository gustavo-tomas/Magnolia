#include "resources/font.hpp"

#include "renderer/renderer.hpp"
#include "resources/resource_loader.hpp"

// @TODO: async loading

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Font>> fonts;
        };

        static State* state = nullptr;

        b8 initialize_font_subsystem()
        {
            state = new State();
            return state != nullptr;
        }

        void shutdown_font_subsystem()
        {
            state->fonts.clear();
            delete state;
        }

        ref<Font> get_font(const str& name)
        {
            auto it = state->fonts.find(name);
            if (it != state->fonts.end())
            {
                return it->second;
            }

            // Create a new font
            Font* font = new Font();
            state->fonts[name] = ref<Font>(font);

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

                    if (texture.width > 0 && texture.height > 0 &&
                        texture.pixels.size() == texture.width * texture.height)
                    {
                        gfx::upload_image(&texture, SamplerAddressMode::ClampToEdge);
                    }
                }
            }

            return state->fonts[name];
        }
    };  // namespace resource
};      // namespace mag
