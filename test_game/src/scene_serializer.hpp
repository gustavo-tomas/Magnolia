#pragma once

#include <magnolia/core/types.hpp>

#include "scene.hpp"

namespace game
{
    namespace scene
    {
        b8 load(const str& file_path, Scene& scene);
        b8 save(const str& file_path, Scene& scene);
    };  // namespace scene
};      // namespace game
