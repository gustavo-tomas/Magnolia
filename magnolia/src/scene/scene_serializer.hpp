#pragma once

#include "scene/scene.hpp"

namespace mag
{
    class SceneSerializer
    {
        public:
            SceneSerializer(BaseScene& scene);
            ~SceneSerializer() = default;

            void serialize(const str& path);
            void deserialize(const str& path);

        private:
            BaseScene& scene;
    };
};  // namespace mag
