#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Material;

    class MaterialLoader
    {
        public:
            b8 load(const str& name, Material* material);
    };
};  // namespace mag
