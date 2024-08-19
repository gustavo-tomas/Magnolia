#pragma once

#include <memory>

#include "imgui.h"
#include "renderer/image.hpp"

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
            std::shared_ptr<Image> folder_image, file_image;
            vk::DescriptorSet folder_image_descriptor = {}, file_image_descriptor = {};
    };
};  // namespace sprout
