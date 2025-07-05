#pragma once

#include <magnolia/core/types.hpp>
#include <magnolia/scene/scene.hpp>

namespace game
{
    namespace scene
    {
        b8 load(const str& file_path, mag::Scene& scene);
        b8 save(const str& file_path, mag::Scene& scene);
    };  // namespace scene
};      // namespace game
