#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Model;

    class ModelLoader
    {
        public:
            b8 load(const str& file_path, Model* model);
    };
};  // namespace mag
