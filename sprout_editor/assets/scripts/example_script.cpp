#include <magnolia.hpp>
#include <math/quat.hpp>

using namespace mag;

mat4 get_rotation_mat(const vec3& rotation)
{
    const quat pitch_rotation = angleAxis(radians(rotation.x), vec3(1, 0, 0));
    const quat yaw_rotation = angleAxis(radians(rotation.y), vec3(0, 1, 0));
    const quat roll_rotation = angleAxis(radians(rotation.z), vec3(0, 0, 1));

    const mat4 rotation_mat = toMat4(roll_rotation) * toMat4(yaw_rotation) * toMat4(pitch_rotation);

    return rotation_mat;
}

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
            if (!transform || !camera_c)
            {
                LOG_WARNING("Missing transform/camera");
                return;
            }

            const mat4 rotation_mat = get_rotation_mat(transform->rotation);
            const vec3 side = rotation_mat[0];
            const vec3 up = rotation_mat[1];
            const vec3 forward = rotation_mat[2];

            vec3 direction(0.0f);
            const f32 speed = 50.0f;

            if (window.is_key_down(Key::a)) direction -= side;
            if (window.is_key_down(Key::d)) direction += side;
            if (window.is_key_down(Key::w)) direction -= forward;
            if (window.is_key_down(Key::s)) direction += forward;
            if (window.is_key_down(Key::Space)) direction += up;
            if (window.is_key_down(Key::Lctrl)) direction -= up;

            // Prevent nan values
            if (length(direction) > 0.0f)
            {
                direction = normalize(direction) * dt;
            }

            auto& camera = camera_c->camera;

            transform->translation += direction * speed;
            camera.set_position(transform->translation);
        }

        virtual void on_event(Event& e) override
        {
            EventDispatcher dispatcher(e);
            dispatcher.dispatch<MouseMoveEvent>(BIND_FN(CameraController::on_mouse_move));
            dispatcher.dispatch<MousePressEvent>(BIND_FN(CameraController::on_mouse_click));
        }

        void on_mouse_click(MousePressEvent& e)
        {
            // Capture/Release the cursor
            if (e.button == Button::Right)
            {
                auto& app = get_application();
                auto& window = app.get_window();

                window.set_capture_mouse(!window.is_mouse_captured());
            }
        }

        void on_mouse_move(MouseMoveEvent& e)
        {
            // This is not as good as updating on the loop with dt, but its a nice example
            auto [transform, camera_c] = get_components<TransformComponent, CameraComponent>();
            if (!transform || !camera_c)
            {
                LOG_WARNING("Missing transform/camera");
                return;
            }

            const ivec2 mouse_dir = {e.x_direction, e.y_direction};

            // Rotate
            transform->rotation += vec3(-mouse_dir.y, -mouse_dir.x, 0.0f) / 10.0f;
            camera_c->camera.set_rotation(transform->rotation);
        }
};

extern "C" ScriptableEntity* create_script() { return new CameraController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
