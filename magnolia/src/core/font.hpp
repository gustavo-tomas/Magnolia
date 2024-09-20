#pragma once

#include <filesystem>

#include "core/types.hpp"

// This implementation was based on the cherno's implementation: https://www.youtube.com/watch?v=iMuiim9loOg

namespace mag
{
    class Font
    {
        public:
            Font(const std::filesystem::path& file_path);
            ~Font();
    };

}  // namespace mag