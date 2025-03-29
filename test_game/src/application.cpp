#include "application.hpp"

#include <core/entry_point.hpp>

mag::Application *mag::create_application() { return new game::TestGame("test_game/config.json"); }

namespace game
{
    TestGame::TestGame(const str &config_file_path) : Application(config_file_path) {}

    TestGame::~TestGame() = default;

    void TestGame::on_update(const f32 dt)
    {
        (void)dt;
        LOG_INFO("Updating game. DT: {0}", dt);
    }

    void TestGame::on_event(const Event &e) { (void)e; }
};  // namespace game
