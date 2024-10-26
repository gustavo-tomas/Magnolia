#pragma once

#include "core/types.hpp"

namespace mag
{
    class Scene;

    class SceneSerializer
    {
        public:
            SceneSerializer(Scene& scene);
            ~SceneSerializer();

            void serialize(const str& file_path);
            void deserialize(const str& file_path);

        private:
            Scene& scene;
    };
};  // namespace mag
