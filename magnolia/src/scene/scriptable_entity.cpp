#include "scene/scriptable_entity.hpp"

#include "core/event.hpp"

namespace mag
{
    ScriptableEntity::ScriptableEntity() = default;
    ScriptableEntity::~ScriptableEntity() = default;

    void ScriptableEntity::on_create() {}
    void ScriptableEntity::on_destroy() {}
    void ScriptableEntity::on_update(const f32 dt) { (void)dt; }
    void ScriptableEntity::on_event(Event& e) { (void)e; }
};  // namespace mag