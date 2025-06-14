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

    struct MaterialResource : public IResource
    {
            std::map<TextureSlot, str> textures;
            str name = "";
    };

    namespace resource
    {
        class MaterialLoader : public IResourceLoader
        {
            public:
                MaterialLoader();
                ~MaterialLoader();

                virtual IResource* load(const str& file_path) override;
        };

        ref<MaterialResource> MAG_API get_default_material();
    };  // namespace resource
};      // namespace mag
