#pragma once

#include <magnolia.hpp>

namespace game
{
    using namespace mag::math;

    class TestGame : public mag::Application
    {
        public:
            TestGame(const str& config_file_path);
            ~TestGame();

            virtual void on_update(const f32 dt) override;
            virtual void on_event(const mag::Event& e) override;

        private:
            unique<mag::Scene> scene = nullptr;
    };
};  // namespace game
