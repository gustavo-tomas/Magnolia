#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    struct ModelResource;
    struct Vertex;

    class MAG_API ModelImporter
    {
        public:
            ModelImporter();
            ~ModelImporter();

            b8 import(const str& file_path, str& imported_model_path);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
