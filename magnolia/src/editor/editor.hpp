#pragma once

#include <memory>

#include "core/window.hpp"
#include "editor/editor_pass.hpp"
#include "imgui.h"

namespace mag
{
    class Editor
    {
        public:
            void initialize(Window& window);
            void shutdown();
            void update(CommandBuffer& cmd, const Image& viewport_image, std::vector<Model>& models);
            void process_events(SDL_Event& e);

            void on_resize(const uvec2& size);
            const Image& get_image() const { return render_pass.get_draw_image(); };
            uvec2 get_draw_size() const { return render_pass.get_draw_size(); };

        private:
            void set_style();

            void render_dummy(const ImGuiWindowFlags window_flags, const str& name);
            void render_panel(const ImGuiWindowFlags window_flags);
            void render_viewport(const ImGuiWindowFlags window_flags, const Image& viewport_image);
            void render_properties(const ImGuiWindowFlags window_flags, std::vector<Model>& models);

            Window* window;
            EditorRenderPass render_pass;
            ImDrawData* draw_data;
            vk::DescriptorPool descriptor_pool;
            vk::DescriptorSet image_descriptor = {}, button_image_descriptor = {};
            std::shared_ptr<Image> button_image;
    };
};  // namespace mag
