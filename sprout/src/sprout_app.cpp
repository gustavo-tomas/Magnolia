#include <core/entry_point.hpp>
#include <magnolia.hpp>
#include <scene/scene_serializer.hpp>  // @TODO: temp

class SproutApp : public mag::Application
{
    public:
        SproutApp(const mag::ApplicationOptions& options) : mag::Application(options)
        {
            mag::Scene* scene = new mag::Scene();
            mag::SceneSerializer scene_serializer(*scene);
            scene_serializer.deserialize("sprout/assets/scenes/test_scene.mag.json");

            this->set_active_scene(scene);
        }
};

mag::Application* mag::create_application()
{
    mag::ApplicationOptions options;
    options.title = "Sprout";

    return new SproutApp(options);
}
