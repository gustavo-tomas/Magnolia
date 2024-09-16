#pragma once

#include <memory>

#include "core/event.hpp"
#include "core/layer.hpp"
#include "menu/menu_bar.hpp"
#include "panels/camera_panel.hpp"
#include "panels/content_browser_panel.hpp"
#include "panels/materials_panel.hpp"
#include "panels/properties_panel.hpp"
#include "panels/scene_panel.hpp"
#include "panels/settings_panel.hpp"
#include "panels/status_panel.hpp"
#include "panels/viewport_panel.hpp"
#include "renderer/render_graph.hpp"
#include "scene/scene.hpp"

namespace sprout
{
    using namespace mag;

    // ImGui drag and drop types
    inline const char* CONTENT_BROWSER_ITEM = "CONTENT_BROWSER_ITEM";

    class Editor : public Layer
    {
        public:
            Editor(const EventCallback& event_callback);
            ~Editor();

            virtual void on_attach() override;
            virtual void on_update(const f32 dt) override;
            virtual void on_event(Event& e) override;

            void add_scene(Scene* scene);

            void set_input_disabled(const b8 disable);
            void set_selected_scene_index(const u32 index);

            // @TODO: this can be extended to query by window name if needed
            b8 is_viewport_window_active() const;

            Scene& get_active_scene() { return *open_scenes[selected_scene_index]; };
            RenderGraph& get_render_graph() { return *render_graph; };
            const std::vector<std::shared_ptr<Scene>>& get_open_scenes() const { return open_scenes; };
            u32 get_selected_scene_index() const { return selected_scene_index; };

            // @TODO: find a better way to pass values to the rest of the application (maybe use a struct?)
            u32& get_texture_output() { return settings_panel->get_texture_output(); };
            u32& get_normal_output() { return settings_panel->get_normal_output(); };
            b8& is_bounding_box_enabled() { return settings_panel->is_bounding_box_enabled(); };
            b8& is_physics_colliders_enabled() { return settings_panel->is_physics_colliders_enabled(); };

            const uvec2& get_viewport_size() const { return viewport_panel->get_viewport_size(); };

            b8 is_disabled() const { return disabled; };

        private:
            friend class EditorPass;

            void on_sdl_event(NativeEvent& e);
            void on_resize(WindowResizeEvent& e);
            void on_viewport_resize(const uvec2& new_viewport_size);

            void set_active_scene(const u32 index);
            void build_render_graph(const uvec2& size, const uvec2& viewport_size);

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

            std::unique_ptr<RenderGraph> render_graph;
            std::vector<std::shared_ptr<Scene>> open_scenes;

            u32 selected_scene_index = 0;
            u32 next_scene_index = 0;

            uvec2 curr_viewport_size;
            b8 disabled = false;
    };

    Editor& get_editor();
};  // namespace sprout
