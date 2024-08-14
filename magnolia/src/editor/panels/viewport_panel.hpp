#pragma once

// clang-format off

#include "camera/camera.hpp"
#include "ecs/ecs.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "renderer/image.hpp"

// clang-format on

namespace mag
{
    class ViewportPanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, const Camera& camera, ECS& ecs,
                        const u32 selected_entity_id, const Image& viewport_image);

            void on_event(Event& e);

            const uvec2& get_viewport_size() const { return viewport_size; };

            b8 is_viewport_window_active() const { return viewport_window_active; };
            b8 is_resize_needed() const { return resize_needed; };

        private:
            void on_key_press(KeyPressEvent& e);

            b8 resize_needed = false;
            b8 viewport_window_active = false;
            uvec2 viewport_size = {1, 1};
            vk::DescriptorSet viewport_image_descriptor = {};
            ImGuizmo::OPERATION gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
    };
};  // namespace mag
