#pragma once

#include "core/math.hpp"
#include "imgui.h"

namespace mag
{
    class RendererImage;
    class Camera;
    class ECS;

    struct Event;
    struct KeyPressEvent;
};  // namespace mag

namespace sprout
{
    using namespace mag;

    class ViewportPanel
    {
        public:
            ViewportPanel();
            ~ViewportPanel();

            void render(const ImGuiWindowFlags window_flags, const Camera& camera, ECS& ecs,
                        const u32 selected_entity_id, const RendererImage& viewport_image);

            void on_event(Event& e);

            const math::uvec2& get_viewport_size() const;
            b8 is_viewport_window_active() const;

        private:
            void on_key_press(KeyPressEvent& e);

            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace sprout
