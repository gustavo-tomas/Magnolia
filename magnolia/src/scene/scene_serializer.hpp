#pragma once

#include "core/types.hpp"

namespace mag
{
    class Scene;

    namespace scene
    {
        MAG_API b8 load(const str& file_path, Scene& scene);
        MAG_API b8 save(const str& file_path, Scene& scene);
    };  // namespace scene
};      // namespace mag
