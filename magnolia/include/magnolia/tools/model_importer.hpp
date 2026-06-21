#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace tools
    {
        MAG_API b8 import_model(const str& file_path, str& imported_model_path);
        MAG_API b8 is_extension_supported(const str& extension_with_dot);
    };  // namespace tools
};  // namespace mag
