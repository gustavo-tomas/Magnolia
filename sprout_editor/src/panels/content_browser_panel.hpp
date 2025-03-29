#pragma once

#include "core/types.hpp"

typedef int ImGuiWindowFlags;

namespace sprout
{
    class ContentBrowserPanel
    {
        public:
            ContentBrowserPanel();
            ~ContentBrowserPanel();

            void render(const ImGuiWindowFlags window_flags);

        private:
            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace sprout
