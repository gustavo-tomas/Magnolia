#pragma once

#include <memory>

#include "core/event.hpp"
#include "core/types.hpp"
#include "editor/menu/info_menu.hpp"
#include "imgui.h"

namespace mag
{
    class MenuBar
    {
        public:
            MenuBar();
            ~MenuBar() = default;

            void render(const ImGuiWindowFlags window_flags);
            b8 quit_requested() const { return quit; };

            void on_event(Event& e);

        private:
            void display_dialog();
            void save_active_scene();
            void save_active_scene_as();
            void quit_application();

            void on_key_press(KeyPressEvent& e);

            std::unique_ptr<InfoMenu> info_menu;
            b8 quit = false;
            b8 dialog_open = false;
            str scene_file_path = "";
    };
};  // namespace mag
