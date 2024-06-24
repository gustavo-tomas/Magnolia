#pragma once

#include "scene/scene.hpp"

namespace mag
{
    class SceneSerializer
    {
        public:
            SceneSerializer(Scene& scene);
            ~SceneSerializer() = default;

            void serialize(const str& file_path);
            void deserialize(const str& file_path);

        private:
            Scene& scene;
    };
};  // namespace mag
