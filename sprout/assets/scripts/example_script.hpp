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

            // @TODO: wrap the other ecs methods
            auto* transform = get_component<TransformComponent>();
            ASSERT(transform, "No transform!");

            auto* camera_c = get_component<CameraComponent>();
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

            const mat4& camera_rot = camera->get_rotation_mat();
            const vec3& camera_position = camera->get_position() + vec3(camera_rot * vec4(direction * speed, 0.0f));
            camera->set_position(camera_position);

            LOG_SUCCESS("POS: {0}", math::to_string(camera_position));
        }
};
