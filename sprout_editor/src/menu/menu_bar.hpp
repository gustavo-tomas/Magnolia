#pragma once

#include "core/event.hpp"
#include "core/types.hpp"
#include "imgui.h"
#include "menu/info_menu.hpp"

namespace sprout
{
    using namespace mag;

    class MenuBar
    {
        public:
            MenuBar();
            ~MenuBar() = default;

            void render(const ImGuiWindowFlags window_flags);
            b8 quit_requested() const { return quit; };

            void on_event(Event& e);

        private:
            void new_scene();
            void save_active_scene();
            void save_active_scene_as();
            void open_scene();
            void quit_application();

            void on_key_press(KeyPressEvent& e);

            unique<InfoMenu> info_menu;
            b8 quit = false;
            str scene_file_path = "";
    };
};  // namespace sprout
