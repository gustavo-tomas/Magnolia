#include <magnolia.hpp>

using namespace mag;

class PhysicsController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created PhysicsController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed PhysicsController"); }

        virtual void on_update(const f32 dt) override
        {
            auto& app = get_application();
            auto& window = get_application().get_window();
            auto& physics = app.get_physics_engine();

            auto [transform, rigidbody] = get_components<TransformComponent, RigidBodyComponent>();
            if (!transform || !rigidbody)
            {
                LOG_WARNING("(PhysicsController) Missing transform/rigidbody");
                return;
            }

            if (window.is_key_pressed(Key::i))
            {
                const vec3 impulse = vec3(0, 2000, 0) * dt;
                physics.apply_impulse(rigidbody->index, impulse);
            }
        }

        virtual void on_event(const Event& e) override { (void)e; }
};

extern "C" ScriptableEntity* create_script() { return new PhysicsController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
