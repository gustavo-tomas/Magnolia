#pragma once

#include <magnolia/core/event.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/project/project.hpp>
#include <magnolia/resources/resource.hpp>

#include "scene.hpp"

namespace game
{
    using namespace mag::math;

    class Renderer;

    class TestGame
    {
        public:
            TestGame();
            ~TestGame();

            void run();

        private:
            void on_update(const f32 dt);
            void on_event(const mag::Event& e);

            void register_commands();

            void on_window_close(const mag::WindowCloseEvent& e);
            void on_quit(const mag::QuitEvent& e);

            b8 running = false;

            mag::unique<Renderer> renderer;
            mag::unique<Scene> scene = nullptr;
            mag::unique<mag::Project> project = nullptr;
    };
};  // namespace game
