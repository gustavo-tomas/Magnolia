#include "resources/font.hpp"

#include "core/application.hpp"

namespace mag
{
#define DEFAULT_FONT_NAME "magnolia/assets/fonts/static/OpenSans-Regular.ttf"

    // @TODO: figure out a way to make this a task.

    ref<Font> FontManager::get(const str& name)
    {
        // Font found
        auto it = fonts.find(name);
        if (it != fonts.end())
        {
            return it->second;
        }

        auto& app = get_application();
        // auto& job_system = app.get_job_system();
        auto& renderer = app.get_renderer();
        auto& font_loader = app.get_font_loader();

        // Create a new font
        Font* font = new Font();

        fonts[name] = ref<Font>(font);

        if (font_loader.load(name, font))
        {
            renderer.upload_image(&font->atlas_image);
        }

        // Send font data to the GPU
        // renderer.upload_image(font);

        // Temporary font to load data into
        // Font* transfer_font = new Font(*font);

        // Load in another thread
        // auto execute = [&font_loader, name, transfer_font]
        // {
        //     // If the load fails we still have valid data
        //     return font_loader.load(name, transfer_font);
        // };

        // // Callback when finished loading
        // auto load_finished_callback = [font, transfer_font, &renderer](const b8 result)
        // {
        //     // Update the font and the renderer font data
        //     if (result == true)
        //     {
        //         *font = *transfer_font;
        //         renderer.update_image(font);
        //     }

        //     // We can dispose of the temporary font now
        //     delete transfer_font;
        // };

        // Job load_job = Job(execute, load_finished_callback);
        // job_system.add_job(load_job);

        return fonts[name];
    }

    ref<Font> FontManager::get_default() { return get(DEFAULT_FONT_NAME); }
}  // namespace mag
