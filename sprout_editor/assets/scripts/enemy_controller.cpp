#include <magnolia.hpp>
#include <math/generic.hpp>

using namespace mag;

class EnemyController : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created EnemyController"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed EnemyController"); }

        // Chase the player
        virtual void on_update(const f32 dt) override
        {
            auto [transform] = get_components<TransformComponent>();
            if (!transform)
            {
                LOG_WARNING("(EnemyController) Missing transform");
                return;
            }

            // Get player position
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

            // Don't come too close
            const f32 dist_to_player = math::distance(transform->translation, target_entity_transform->translation);
            if (dist_to_player <= 20.0f)
            {
                add_entity_to_deletion_queue();
                return;
            }

            // Rotate towards the player. We add a tiny modifier to prevent nan results.
            mat4 inv_view_mat =
                math::lookAt(transform->translation + vec3(1e-7), target_entity_transform->translation, vec3(0, 1, 0));

            inv_view_mat = math::inverse(inv_view_mat);

            const vec3 new_rotation = math::eulerAngles(math::toQuat(inv_view_mat));
            const vec3 forward = normalize(inv_view_mat[2]);

            // Prevent nan values
            const vec3 is_nan = math::isnan(forward);
            if (is_nan.x || is_nan.y || is_nan.z)
            {
                return;
            }

            // Move towards the player
            const f32 speed = 10.0f;
            const vec3 impulse = forward * speed * dt;

            transform->translation -= impulse;
            transform->rotation = new_rotation;
        }

        virtual void on_event(const Event& e) override { (void)e; }

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

extern "C" ScriptableEntity* create_script() { return new EnemyController(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
