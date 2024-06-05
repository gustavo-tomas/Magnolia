#pragma once

// clang-format off

#include <memory>
#include "core/event.hpp"
#include "editor/editor_pass.hpp"
#include "ecs/ecs.hpp"
#include "editor/panels/content_browser.hpp"
#include "imgui.h"
#include "ImGuizmo.h"

// clang-format on

namespace mag
{
    // ImGui drag and drop types
    inline const char* CONTENT_BROWSER_ITEM = "CONTENT_BROWSER_ITEM";

    class Editor
    {
        public:
            Editor(const EventCallback& event_callback);
            ~Editor();

            void update();
            void render(Camera& camera, ECS& ecs);
            void on_event(Event& e);

            void set_viewport_image(const Image& image);
            void set_input_disabled(const b8 disable);

            // @TODO: this can be extended to query by window name if needed
            const b8& is_viewport_window_active() const { return viewport_window_active; };

            u32& get_texture_output() { return texture_output; };
            u32& get_normal_output() { return normal_output; };
            const Image& get_image() const { return render_pass.get_draw_image(); };
            const uvec3& get_draw_size() const { return render_pass.get_draw_size(); };

        private:
            void on_sdl_event(SDLEvent& e);
            void on_resize(WindowResizeEvent& e);
            void on_key_press(KeyPressEvent& e);

            void set_style();

            void render_panel(const ImGuiWindowFlags window_flags);
            void render_viewport(const ImGuiWindowFlags window_flags, const Camera& camera, ECS& ecs);
            void render_scene(const ImGuiWindowFlags window_flags, ECS& ecs);
            void render_settings(const ImGuiWindowFlags window_flags);
            void render_properties(ECS& ecs, const u32 entity_id);
            void render_materials(const ModelComponent& model_component);

            // @TODO: this is temporary-ish. The camera will (should?) be an ECS component.
            void render_camera_properties(const ImGuiWindowFlags window_flags, Camera& camera);

            void render_status(const ImGuiWindowFlags window_flags);

            EventCallback event_callback;

            std::unique_ptr<ContentBrowserPanel> content_browser_panel;

            EditorRenderPass render_pass;
            ImDrawData* draw_data;
            vk::DescriptorPool descriptor_pool;
            vk::DescriptorSet viewport_image_descriptor = {};

            const Image* viewport_image = {};

            uvec2 viewport_size = {1, 1};
            b8 resize_needed = false;
            b8 disabled = false;
            b8 viewport_window_active = false;
            u32 texture_output = 0, normal_output = 0;
            u64 selected_entity_id = INVALID_ID;
            ImGuizmo::OPERATION gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
    };
};  // namespace mag
