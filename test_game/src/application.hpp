#pragma once

#include <magnolia.hpp>

namespace game
{
    using namespace mag;

    class TestGame : public Application
    {
        public:
            TestGame(const str& config_file_path);
            ~TestGame();

            virtual void on_update(const f32 dt) override;
            virtual void on_event(const Event& e) override;
    };
};  // namespace game
