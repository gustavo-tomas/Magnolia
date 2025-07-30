#pragma once

#include <magnolia/core/event.hpp>
#include <magnolia/core/types.hpp>
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

            // -1 is no limits
            void set_target_frame_rate(const f32 frame_rate);

            void on_window_close(const mag::WindowCloseEvent& e);
            void on_quit(const mag::QuitEvent& e);

            b8 running = false;
            f32 target_frame_rate = -1;

            mag::unique<Renderer> renderer;
            mag::unique<Scene> scene = nullptr;
    };
};  // namespace game
