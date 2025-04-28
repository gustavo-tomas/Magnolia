#include "resources/font.hpp"

#include "renderer/renderer.hpp"
#include "resources/resource.hpp"
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
            font->loading_status = LoadingStatus::InProgress;
            state->fonts[name] = ref<Font>(font);

            // Upload character texture to the GPU
            if (resource::load(name, font))
            {
                font->loading_status = LoadingStatus::Finished;
                for (auto& [c, character] : font->characters)
                {
                    if (!character.data.empty())
                    {
                        gfx::upload_image(&character.texture, SamplerAddressMode::ClampToEdge);
                    }
                }
                font->loading_status = LoadingStatus::UploadedToGpu;
            }

            else
            {
                font->loading_status = LoadingStatus::Error;
            }

            return state->fonts[name];
        }
    };  // namespace resource
};      // namespace mag
