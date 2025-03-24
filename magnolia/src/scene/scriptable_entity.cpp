#include "scene/scriptable_entity.hpp"

#include "core/event.hpp"
#include "scene/scene.hpp"

namespace mag
{
    ScriptableEntity::ScriptableEntity() = default;
    ScriptableEntity::~ScriptableEntity() = default;

    void ScriptableEntity::on_create() {}
    void ScriptableEntity::on_destroy() {}
    void ScriptableEntity::on_update(const f32 dt) { (void)dt; }
    void ScriptableEntity::on_event(const Event& e) { (void)e; }

    void ScriptableEntity::add_entity_to_deletion_queue() { scene->remove_entity(entity_id); }

    u32 ScriptableEntity::create_entity(const str& name) const { return ecs->create_entity(name); }

    PhysicsWorld& ScriptableEntity::get_physics_world() const { return *physics_world; }
};  // namespace mag
