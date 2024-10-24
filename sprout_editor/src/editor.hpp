#pragma once

#include <magnolia.hpp>

namespace mag
{
    class RenderGraph;
    class RendererImage;
}  // namespace mag

namespace sprout
{
    using namespace mag;

    // ImGui drag and drop types
    inline const c8* CONTENT_BROWSER_ITEM = "CONTENT_BROWSER_ITEM";

    class EditorScene;

    class Editor : public Application
    {
        public:
            Editor(const str& config_file_path);
            ~Editor();

            virtual void on_update(const f32 dt) override;
            virtual void on_event(Event& e) override;

            void add_scene(EditorScene* scene);
            void close_scene(const ref<EditorScene>& scene);

            void set_input_disabled(const b8 disable);
            void set_selected_scene_index(const u32 index);

            // @TODO: this can be extended to query by window name if needed
            b8 is_viewport_window_active() const;

            EditorScene& get_active_scene();
            RenderGraph& get_render_graph();
            const std::vector<ref<EditorScene>>& get_open_scenes() const;
            const uvec2& get_viewport_size() const;
            u32 get_selected_scene_index() const;

            // @TODO: find a better way to pass values to the rest of the application (maybe use a struct?)
            u32& get_texture_output();
            u32& get_normal_output();
            b8& is_bounding_box_enabled();
            b8& is_physics_colliders_enabled();
            b8 is_disabled() const;

        private:
            friend class EditorPass;

            void render(ECS& ecs, Camera& camera, RendererImage& viewport_image);

            void on_sdl_event(NativeEvent& e);
            void on_resize(WindowResizeEvent& e);
            void on_quit(QuitEvent& e);
            void on_viewport_resize(const uvec2& new_viewport_size);

            void set_active_scene(const u32 index);
            void build_render_graph(const uvec2& size, const uvec2& viewport_size);

            struct IMPL;
            unique<IMPL> impl;
    };

    Editor& get_editor();
};  // namespace sprout
