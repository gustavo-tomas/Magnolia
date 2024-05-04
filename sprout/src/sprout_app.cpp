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

    // @TODO: Create a scene system to decouple the cube scene from the engine

    return new SproutApp(options);
}
