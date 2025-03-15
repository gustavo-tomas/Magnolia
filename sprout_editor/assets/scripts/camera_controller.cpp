#include <magnolia.hpp>
#include <math/quat.hpp>
#include <math/types.hpp>

using namespace mag;

class CameraController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created CameraController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed CameraController"); }

        // Update the camera to follow the player
        virtual void on_update(const f32 dt) override
        {
            (void)dt;

            // Check for missing components
            auto [camera_transform, camera_c] = get_components<TransformComponent, CameraComponent>();
            if (!camera_transform || !camera_c)
            {
                LOG_WARNING("(CameraController) Missing transform/camera");
                return;
            }

            Camera& camera = camera_c->camera;

            // @TODO: a better system would use tags instead of searching by name
            if (target_entity_id == Invalid_ID)
            {
                find_target_by_name("Player");
            }

            auto [target_entity_transform] = get_external_entity_components<TransformComponent>(target_entity_id);

            if (!target_entity_transform)
            {
                return;
            }

            // Update the transform
            camera_transform->translation = target_entity_transform->translation;
            camera_transform->rotation = target_entity_transform->rotation;

            camera.set_position(camera_transform->translation);
            camera.set_rotation(camera_transform->rotation);
        }

        void follow_entity(const u32 id) { target_entity_id = id; }

    private:
        void find_target_by_name(const str& name)
        {
            const std::vector<u32> entity_ids = get_entities_with_components_of_type<NameComponent>();
            for (const u32 id : entity_ids)
            {
                auto [name_component] = get_external_entity_components<NameComponent>(id);
                if (name_component && name_component->name == name)
                {
                    target_entity_id = id;
                    break;
                }
            }
        }

        u32 target_entity_id = Invalid_ID;
};

extern "C" ScriptableEntity* create_script() { return new CameraController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
