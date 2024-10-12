#pragma once

#include <magnolia.hpp>

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
    inline const c8* CONTENT_BROWSER_ITEM = "CONTENT_BROWSER_ITEM";

    class Editor : public Application
    {
        public:
            Editor(const mag::ApplicationOptions& options);
            ~Editor();

            virtual void on_update(const f32 dt) override;
            virtual void on_event(Event& e) override;

            void add_scene(Scene* scene);
            void close_scene(const ref<Scene>& scene);

            void set_input_disabled(const b8 disable);
            void set_selected_scene_index(const u32 index);

            // @TODO: this can be extended to query by window name if needed
            b8 is_viewport_window_active() const;

            Scene& get_active_scene() { return *open_scenes[selected_scene_index]; };
            RenderGraph& get_render_graph() { return *render_graph; };
            const std::vector<ref<Scene>>& get_open_scenes() const { return open_scenes; };
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
            void on_quit(QuitEvent& e);
            void on_viewport_resize(const uvec2& new_viewport_size);

            void set_active_scene(const u32 index);
            void build_render_graph(const uvec2& size, const uvec2& viewport_size);

            vk::DescriptorPool descriptor_pool;

            unique<MenuBar> menu_bar;
            unique<ContentBrowserPanel> content_browser_panel;
            unique<ViewportPanel> viewport_panel;
            unique<ScenePanel> scene_panel;
            unique<MaterialsPanel> material_panel;
            unique<PropertiesPanel> properties_panel;
            unique<StatusPanel> status_panel;
            unique<CameraPanel> camera_panel;
            unique<SettingsPanel> settings_panel;

            unique<RenderGraph> render_graph;
            std::vector<ref<Scene>> open_scenes;
            std::vector<u32> open_scenes_marked_for_deletion;

            u32 selected_scene_index = 0;
            u32 next_scene_index = 0;

            uvec2 curr_viewport_size;
            b8 disabled = false;
    };

    Editor& get_editor();
};  // namespace sprout
