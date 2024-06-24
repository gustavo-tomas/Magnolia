#pragma once

#include "scene/scene.hpp"

namespace mag
{
    class SceneSerializer
    {
        public:
            SceneSerializer(BaseScene& scene);
            ~SceneSerializer() = default;

            void serialize(const str& file_path);
            void deserialize(const str& file_path);

        private:
            BaseScene& scene;
    };
};  // namespace mag
