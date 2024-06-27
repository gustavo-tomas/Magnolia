#pragma once

#include <magnolia.hpp>

using namespace mag;

class ExampleScript : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed"); }

        virtual void on_update(const f32 dt) override
        {
            auto& app = get_application();
            auto& window = app.get_window();

            auto* transform = get_component<TransformComponent>();
            ASSERT(transform, "No transform!");

            f32 speed = 50.0f;

            if (window.is_key_down(Key::w)) transform->translation.x += speed * dt;
            if (window.is_key_down(Key::s)) transform->translation.x -= speed * dt;
            if (window.is_key_down(Key::a)) transform->translation.z -= speed * dt;
            if (window.is_key_down(Key::d)) transform->translation.z += speed * dt;
        }
};
