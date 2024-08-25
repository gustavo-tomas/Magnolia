#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Material;

    class MaterialLoader
    {
        public:
            Material* load(const str& name);
    };
};  // namespace mag
