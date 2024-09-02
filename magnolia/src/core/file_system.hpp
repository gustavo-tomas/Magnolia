#pragma once

#include <filesystem>

#include "core/buffer.hpp"
#include "core/types.hpp"
#include "nlohmann/json.hpp"

// Also see the cherno implementation:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/FileSystem.cpp

namespace mag
{
    using json = nlohmann::ordered_json;

    class FileSystem
    {
        public:
            b8 read_binary_data(const std::filesystem::path& file_path, Buffer& buffer);
            b8 write_binary_data(const std::filesystem::path& file_path, Buffer& buffer);

            b8 read_json_data(const std::filesystem::path& file_path, json& data);
    };
};  // namespace mag
