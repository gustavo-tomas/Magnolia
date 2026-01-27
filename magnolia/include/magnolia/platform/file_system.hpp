#pragma once

#include "magnolia/core/types.hpp"
#include "nlohmann/json.hpp"

// @TODO: figure out how to handle file paths (see shader.cpp)

namespace mag
{
    struct Buffer;

    namespace fs
    {
        using path = std::filesystem::path;
        using json = nlohmann::ordered_json;

        b8 initialize();
        void shutdown();

        MAG_API b8 read_binary_data(const fs::path& file_path, Buffer& buffer);
        MAG_API b8 write_binary_data(const fs::path& file_path, Buffer& buffer);

        MAG_API b8 read_json_data(const fs::path& file_path, fs::json& data);
        MAG_API b8 write_json_data(const fs::path& file_path, const fs::json& data);

        MAG_API b8 create_directories(const fs::path& path);

        MAG_API str get_file_extension(const fs::path& file_path);
        MAG_API str get_file_name(const fs::path& file_path);
        MAG_API fs::path get_fixed_path(const fs::path& file_path);

        MAG_API b8 exists(const fs::path& path);
        MAG_API b8 is_directory(const fs::path& path);

        // File watcher stuff
        void watch_file(const fs::path& file_path);
        void stop_watching_file(const fs::path& file_path);
        void reset_file_status(const fs::path& file_path);

        b8 was_file_modified(const fs::path& file_path);
    };  // namespace fs
};  // namespace mag
