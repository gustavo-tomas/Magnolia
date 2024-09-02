#pragma once

#include <filesystem>

#include "core/buffer.hpp"
#include "core/types.hpp"

// Also see the cherno implementation:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/FileSystem.cpp

namespace mag
{
    class FileSystem
    {
        public:
            b8 read_binary_data(const std::filesystem::path& file_path, Buffer& buffer);
    };
};  // namespace mag
