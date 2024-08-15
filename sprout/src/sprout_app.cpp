#include <core/entry_point.hpp>
#include <magnolia.hpp>

#include "editor/editor.hpp"

class SproutApp : public mag::Application
{
    public:
        SproutApp(const mag::ApplicationOptions& options) : mag::Application(options)
        {
            push_layer(new mag::Editor(BIND_FN(Application::on_event)));
        }
};

mag::Application* mag::create_application()
{
    mag::ApplicationOptions options;
    options.title = "Sprout";

    return new SproutApp(options);
}
