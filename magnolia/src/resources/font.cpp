#include "resources/font.hpp"

#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Font>> fonts;
                ResourceLoadedCallbackFn on_resource_loaded;
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

            // Temporary font to load data into
            Font* transfer_font = new Font(*font);

            // Load in another thread
            auto execute = [name, transfer_font]
            {
                const b8 result = resource::load(name, transfer_font);

                if (result)
                {
                    transfer_font->loading_status = LoadingStatus::Finished;
                }

                else
                {
                    transfer_font->loading_status = LoadingStatus::Error;
                }

                return result;
            };

            // Callback when finished loading
            auto load_finished_callback = [font, transfer_font](const b8 result)
            {
                // Update the font data
                if (result == true)
                {
                    *font = *transfer_font;
                    state->on_resource_loaded(font);
                }

                // We can dispose of the temporary font now
                delete transfer_font;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->fonts[name];
        }

        void set_on_font_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_resource_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
