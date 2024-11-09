#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Model;
    struct Vertex;

    class ModelImporter
    {
        public:
            ModelImporter();
            ~ModelImporter();

            b8 import(const str& name, str& imported_model_path);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
