#pragma once

#include <memory>

#include "core/event.hpp"
#include "editor/menu/menu_bar.hpp"
#include "editor/panels/camera_panel.hpp"
#include "editor/panels/content_browser_panel.hpp"
#include "editor/panels/materials_panel.hpp"
#include "editor/panels/properties_panel.hpp"
#include "editor/panels/scene_panel.hpp"
#include "editor/panels/settings_panel.hpp"
#include "editor/panels/status_panel.hpp"
#include "editor/panels/viewport_panel.hpp"
#include "renderer/render_graph.hpp"

namespace mag
{
    // ImGui drag and drop types
    inline const char* CONTENT_BROWSER_ITEM = "CONTENT_BROWSER_ITEM";

    class EditorPass : public RenderGraphPass
    {
        public:
            EditorPass(const uvec2& size);

            virtual void on_render(RenderGraph& render_graph) override;
    };

    class Editor
    {
        public:
            Editor(const EventCallback& event_callback);
            ~Editor();

            void update();
            void on_event(Event& e);

            void set_input_disabled(const b8 disable);

            // @TODO: this can be extended to query by window name if needed
            b8 is_viewport_window_active() const;

            u32& get_texture_output() { return settings_panel->get_texture_output(); };
            u32& get_normal_output() { return settings_panel->get_normal_output(); };
            const uvec2& get_viewport_size() const { return viewport_panel->get_viewport_size(); };

            b8 is_disabled() const { return disabled; };

        private:
            friend class EditorPass;

            void on_sdl_event(SDLEvent& e);

            EventCallback event_callback;
            vk::DescriptorPool descriptor_pool;

            std::unique_ptr<MenuBar> menu_bar;
            std::unique_ptr<ContentBrowserPanel> content_browser_panel;
            std::unique_ptr<ViewportPanel> viewport_panel;
            std::unique_ptr<ScenePanel> scene_panel;
            std::unique_ptr<MaterialsPanel> material_panel;
            std::unique_ptr<PropertiesPanel> properties_panel;
            std::unique_ptr<StatusPanel> status_panel;
            std::unique_ptr<CameraPanel> camera_panel;
            std::unique_ptr<SettingsPanel> settings_panel;

            b8 disabled = false;
    };
};  // namespace mag
