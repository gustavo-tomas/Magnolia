#pragma once

#include <map>

#include "core/types.hpp"
#include "resources/resource.hpp"

namespace mag
{
#define DEFAULT_MATERIAL_NAME "__mag_default_material__"

    enum class TextureSlot
    {
        Albedo = 0,
        Normal,
        Roughness,
        Metalness,

        TextureCount
    };

    struct Material : public IResource
    {
            std::map<TextureSlot, str> textures;
            str name = "";
    };

    namespace resource
    {
        ref<Material> get_material(const str& name);
        ref<Material> get_default_material();
    };  // namespace resource
};      // namespace mag
