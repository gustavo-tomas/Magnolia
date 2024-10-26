#pragma once

#include "core/types.hpp"

typedef int ImGuiWindowFlags;

namespace mag
{
    struct Event;
    struct KeyPressEvent;
};  // namespace mag

namespace sprout
{
    using namespace mag;

    class InfoMenu;

    class MenuBar
    {
        public:
            MenuBar();
            ~MenuBar();

            void render(const ImGuiWindowFlags window_flags);
            void on_event(Event& e);

            b8 quit_requested() const;

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
