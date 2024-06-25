#include <magnolia.hpp>

class SproutApp : public mag::Application
{
    public:
        SproutApp(const mag::ApplicationOptions& options) : mag::Application(options)
        {
            this->set_active_scene(new mag::Scene());
        }
};

mag::Application* mag::create_application()
{
    mag::ApplicationOptions options;
    options.title = "Sprout";

    return new SproutApp(options);
}
