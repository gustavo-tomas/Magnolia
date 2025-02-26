#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

typedef int ImGuiWindowFlags;

namespace mag
{
    class ECS;
};

namespace sprout
{
    using namespace mag;

    class PropertiesPanel
    {
        public:
            PropertiesPanel();
            ~PropertiesPanel();

            void render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id);

        private:
            b8 editable_field(const str &field_name, math::vec3 &value, const math::vec3 &reset_value,
                              const math::vec3 &min_value, const math::vec3 &max_value) const;

            b8 editable_field(const str &field_name, f32 &value, const f32 reset_value, const f32 min_value,
                              const f32 max_value) const;
    };
};  // namespace sprout
