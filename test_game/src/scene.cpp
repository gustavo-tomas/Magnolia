#include "scene.hpp"

#include <magnolia/audio/audio_system.hpp>
#include <magnolia/camera/camera.hpp>
#include <magnolia/core/assert.hpp>
#include <magnolia/core/event.hpp>
#include <magnolia/math/types.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/scripting/scripting_engine.hpp>

#include "components.hpp"

namespace game
{
    Scene::Scene() : ecs(mag::create_unique<mag::ECS>(BIND_FN2(Scene::on_component_added))) {}

    Scene::~Scene()
    {
        if (running)
        {
            on_stop();
        }
    }

    void Scene::on_start() { running = true; }

    void Scene::on_stop() { running = false; }

    void Scene::on_update(const f32 dt) { (void)dt; }

    void Scene::on_component_added(const mag::EntityID id, std::any component)
    {
        (void)id;
        (void)component;
    }

    void Scene::on_event(const mag::Event& e) { dispatch_event<mag::WindowResizeEvent>(e, BIND_FN(Scene::on_resize)); }

    void Scene::on_resize(const mag::WindowResizeEvent& e)
    {
        const uvec2& size = {e.width, e.height};

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_viewport_size(size);
        }
    }

    b8 Scene::is_running() const { return running; }

    mag::ECS& Scene::get_ecs() { return *ecs; }

    mag::Camera& Scene::get_camera()
    {
        // @TODO: for now we assume the active camera is the first entity with a camera component
        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            return camera_c->camera;
        }

        MAG_ASSERT(false, "No runtime camera!");
        return std::get<0>(components[0])->camera;
    }
};  // namespace game
