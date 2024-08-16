#pragma once

#include "ecs/ecs.hpp"
#include "imgui.h"

namespace sprout
{
    using namespace mag;

    class PropertiesPanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id);

        private:
            void editable_field(const str &field_name, vec3 &value, const vec3 &reset_value, const vec3 &min_value,
                                const vec3 &max_value);

            void editable_field(const str &field_name, f32 &value, const f32 reset_value, const f32 min_value,
                                const f32 max_value);
    };
};  // namespace sprout
