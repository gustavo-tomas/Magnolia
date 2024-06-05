#pragma once

#include <memory>

#include "core/event.hpp"
#include "ecs/ecs.hpp"
#include "editor/editor_pass.hpp"
#include "editor/panels/content_browser_panel.hpp"
#include "editor/panels/info_panel.hpp"
#include "editor/panels/materials_panel.hpp"
#include "editor/panels/properties_panel.hpp"
#include "editor/panels/scene_panel.hpp"
#include "editor/panels/viewport_panel.hpp"
#include "imgui.h"

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
            b8 is_viewport_window_active() const;

            u32& get_texture_output() { return texture_output; };
            u32& get_normal_output() { return normal_output; };
            const Image& get_image() const { return render_pass.get_draw_image(); };
            const uvec3& get_draw_size() const { return render_pass.get_draw_size(); };

            b8 is_disabled() const { return disabled; };

        private:
            void on_sdl_event(SDLEvent& e);
            void on_resize(WindowResizeEvent& e);

            void render_settings(const ImGuiWindowFlags window_flags);

            // @TODO: this is temporary-ish. The camera will (should?) be an ECS component.
            void render_camera_properties(const ImGuiWindowFlags window_flags, Camera& camera);

            void render_status(const ImGuiWindowFlags window_flags);

            EventCallback event_callback;

            std::unique_ptr<ContentBrowserPanel> content_browser_panel;
            std::unique_ptr<ViewportPanel> viewport_panel;
            std::unique_ptr<InfoPanel> info_panel;
            std::unique_ptr<ScenePanel> scene_panel;
            std::unique_ptr<MaterialsPanel> material_panel;
            std::unique_ptr<PropertiesPanel> properties_panel;

            EditorRenderPass render_pass;
            vk::DescriptorPool descriptor_pool;

            b8 disabled = false;
            u32 texture_output = 0, normal_output = 0;
            u64 selected_entity_id = INVALID_ID;
    };
};  // namespace mag
