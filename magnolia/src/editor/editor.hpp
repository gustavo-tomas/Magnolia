#pragma once

#include "core/window.hpp"
#include "editor/editor_pass.hpp"
#include "imgui.h"

namespace mag
{
    class Editor
    {
        public:
            void initialize();
            void shutdown();
            void update(CommandBuffer& cmd, const Image& viewport_image, std::vector<Model>& models);
            void process_events(SDL_Event& e);

            void on_resize(const uvec2& size);
            const Image& get_image() const { return render_pass.get_draw_image(); };
            uvec2 get_draw_size() const { return render_pass.get_draw_size(); };

        private:
            void set_style();

            EditorRenderPass render_pass;
            ImDrawData* draw_data;
            vk::DescriptorPool descriptor_pool;
            vk::DescriptorSet image_descriptor = {};
    };
};  // namespace mag
