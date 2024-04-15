#pragma once

#include <memory>

#include "editor/editor_pass.hpp"
#include "imgui.h"

namespace mag
{
    class Editor
    {
        public:
            void initialize();
            void shutdown();
            void update();
            void render(CommandBuffer& cmd, std::vector<Model>& models, const Camera& camera);
            void process_events(SDL_Event& e);

            void on_resize(const uvec2& size);
            void on_viewport_resize(std::function<void(const uvec2&)> callback);

            void set_viewport_image(const Image& image);
            void set_input_disabled(const b8 disable);
            const b8& is_input_disabled() const { return disabled; };
            const Image& get_image() const { return render_pass.get_draw_image(); };
            uvec2 get_draw_size() const { return render_pass.get_draw_size(); };

        private:
            void set_style();

            void render_content_browser(const ImGuiWindowFlags window_flags);
            void render_panel(const ImGuiWindowFlags window_flags);
            void render_viewport(const ImGuiWindowFlags window_flags, std::vector<Model>& models, const Camera& camera);
            void render_scene(const ImGuiWindowFlags window_flags, std::vector<Model>& models);
            void render_properties(const ImGuiWindowFlags window_flags, Model* model = nullptr);

            std::function<void(const vec2&)> viewport_resize = {};

            EditorRenderPass render_pass;
            ImDrawData* draw_data;
            vk::DescriptorPool descriptor_pool;
            vk::DescriptorSet image_descriptor = {}, button_image_descriptor = {};
            std::shared_ptr<Image> button_image;
            const Image* viewport_image = {};
            uvec2 viewport_size = {1, 1};
            b8 resize_needed = false;
            b8 disabled = false;
            u64 selected_model_idx = std::numeric_limits<u64>().max();
    };
};  // namespace mag
