#pragma once

#include "imgui.h"
#include "renderer/renderer_image.hpp"

namespace sprout
{
    using namespace mag;

    class ContentBrowserPanel
    {
        public:
            ContentBrowserPanel();
            ~ContentBrowserPanel() = default;

            void render(const ImGuiWindowFlags window_flags);

        private:
            ref<RendererImage> folder_image, file_image;
            vk::DescriptorSet folder_image_descriptor = {}, file_image_descriptor = {};
    };
};  // namespace sprout
