#include <magnolia.hpp>

using namespace mag;

class PhysicsController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created PhysicsController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed PhysicsController"); }

        virtual void on_update(const f32 dt) override
        {
            Application& app = get_application();
            Window& window = app.get_window();
            PhysicsWorld& physics_world = get_physics_world();

            auto [transform, rigidbody] = get_components<TransformComponent, RigidBodyComponent>();
            if (!transform || !rigidbody)
            {
                LOG_WARNING("(PhysicsController) Missing transform/rigidbody");
                return;
            }

            if (window.is_key_pressed(Key::i))
            {
                const vec3 impulse = vec3(0, 2000, 0) * dt;
                physics_world.apply_torque_impulse(rigidbody->collision_object, impulse);
            }
        }

        virtual void on_event(const Event& e) override { (void)e; }
};

extern "C" ScriptableEntity* create_script() { return new PhysicsController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
