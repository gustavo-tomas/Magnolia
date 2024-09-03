#pragma once

#include <filesystem>

#include "core/buffer.hpp"
#include "core/types.hpp"
#include "nlohmann/json.hpp"

// Also see the cherno implementation:
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/FileSystem.cpp

// @TODO: figure out how to handle file paths (see shader.cpp)

namespace mag
{
    using json = nlohmann::ordered_json;

    class FileSystem
    {
        public:
            b8 read_binary_data(const std::filesystem::path& file_path, Buffer& buffer) const;
            b8 write_binary_data(const std::filesystem::path& file_path, Buffer& buffer) const;

            b8 read_json_data(const std::filesystem::path& file_path, json& data) const;
            b8 write_json_data(const std::filesystem::path& file_path, json& data) const;

            b8 create_directories(const std::filesystem::path& path) const;

            str get_file_extension(const std::filesystem::path& file_path) const;

            b8 exists(const std::filesystem::path& path) const;
            b8 is_directory(const std::filesystem::path& path) const;
    };
};  // namespace mag
