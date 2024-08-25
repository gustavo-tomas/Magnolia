#pragma once

#include <map>
#include <memory>

#include "core/types.hpp"

namespace mag
{
    enum class TextureSlot
    {
        Albedo = 0,
        Normal,

        TextureCount
    };

    struct MaterialResource
    {
            std::map<TextureSlot, str> textures;
    };

    class MaterialLoader
    {
        public:
            std::shared_ptr<MaterialResource> load(const str& name);
            void unload(const str& name);

        private:
            std::map<str, std::shared_ptr<MaterialResource>> material_resources;
    };
};  // namespace mag
