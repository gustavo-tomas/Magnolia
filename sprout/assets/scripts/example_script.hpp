#pragma once

#include <magnolia.hpp>

using namespace mag;

class CameraController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created CameraController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed CameraController"); }

        virtual void on_update(const f32 dt) override
        {
            auto& app = get_application();
            auto& window = app.get_window();

            auto [transform, camera_c] = get_components<TransformComponent, CameraComponent>();
            ASSERT(transform, "No transform!");
            ASSERT(camera_c, "No camera!");

            auto& camera = camera_c->camera;

            vec3 direction(0.0f);
            const f32 speed = 50.0f;

            if (window.is_key_down(Key::a)) direction.x -= 1.0f;
            if (window.is_key_down(Key::d)) direction.x += 1.0f;
            if (window.is_key_down(Key::w)) direction.z -= 1.0f;
            if (window.is_key_down(Key::s)) direction.z += 1.0f;
            if (window.is_key_down(Key::Space)) direction.y += 1.0f;
            if (window.is_key_down(Key::Lctrl)) direction.y -= 1.0f;

            // Prevent nan values
            if (length(direction) > 0.0f) direction = normalize(direction) * dt;

            transform->translation += direction * speed;
            camera->set_position(transform->translation);

            LOG_SUCCESS("POS: {0}", math::to_string(camera->get_position()));
        }
};
