#pragma once

#include <map>
#include <memory>

#include "core/types.hpp"

namespace mag
{
#define DEFAULT_MATERIAL_NAME "sprout_editor/assets/materials/default_material.mat.json"

    enum class TextureSlot
    {
        Albedo = 0,
        Normal,

        TextureCount
    };

    // @TODO: this enum is mainly to signal shaders when a material is finished loading. Descriptors may be different
    // for each shader so we create them in the shader class. Of course when a material finishes loading we need
    // to update the descriptors, so we use this flag to signal when a descriptor should be updated. I think its a bit
    // of a hack. idk. do smth about it.
    enum class MaterialLoadingState
    {
        NotLoaded,
        LoadingInProgress,
        LoadingFinished,
        UploadedToGPU
    };

    struct Material
    {
            std::map<TextureSlot, str> textures;
            str name = "";

            MaterialLoadingState loading_state = MaterialLoadingState::NotLoaded;
    };

    class MaterialManager
    {
        public:
            ref<Material> get(const str& name);
            ref<Material> get_default();

        private:
            std::map<str, ref<Material>> materials;
    };
};  // namespace mag
