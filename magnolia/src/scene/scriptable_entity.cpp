#include "magnolia/scene/scriptable_entity.hpp"

#include "magnolia/core/event.hpp"
#include "magnolia/scene/scene.hpp"

namespace mag
{
    ScriptableEntity::ScriptableEntity() = default;
    ScriptableEntity::~ScriptableEntity() = default;

    void ScriptableEntity::on_create() {}
    void ScriptableEntity::on_destroy() {}
    void ScriptableEntity::on_update(const f32 dt) { (void)dt; }
    void ScriptableEntity::on_event(const Event& e) { (void)e; }

    void ScriptableEntity::on_signal_sent(const u32 target_id, const void* data)
    {
        ScriptComponent* script = ecs->get_component<ScriptComponent>(target_id);

        if (!script || !script->entity)
        {
            return;
        }

        // Send signal to target entity
        script->entity->on_signal_received(entity_id, data);
    }

    void ScriptableEntity::on_signal_received(const u32 sender_id, const void* data)
    {
        (void)sender_id;
        (void)data;
    }

    void ScriptableEntity::add_entity_to_deletion_queue() { scene->remove_entity(entity_id); }

    void ScriptableEntity::set_active_scene(const str& scene_file_path) { scene->set_next_scene(scene_file_path); }

    u32 ScriptableEntity::create_entity(const str& name) const { return ecs->create_entity(name); }

    IPhysicsWorld& ScriptableEntity::get_physics_world() const { return *physics_world; }
};  // namespace mag
