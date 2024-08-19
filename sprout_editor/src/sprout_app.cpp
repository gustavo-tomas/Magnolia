#include <core/entry_point.hpp>
#include <magnolia.hpp>

#include "editor.hpp"

namespace sprout
{
    class SproutApp : public mag::Application
    {
        public:
            SproutApp(const mag::ApplicationOptions& options) : mag::Application(options)
            {
                push_layer(new Editor(BIND_FN(Application::on_event)));
            }
    };
};  // namespace sprout

mag::Application* mag::create_application()
{
    mag::ApplicationOptions options;
    options.title = "Sprout";

    return new sprout::SproutApp(options);
}
