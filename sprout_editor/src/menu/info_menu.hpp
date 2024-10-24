#pragma once

typedef int ImGuiWindowFlags;

namespace sprout
{
    class InfoMenu
    {
        public:
            InfoMenu();
            ~InfoMenu();

            void render(const ImGuiWindowFlags window_flags);
    };
};  // namespace sprout
