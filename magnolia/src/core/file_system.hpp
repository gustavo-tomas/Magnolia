#pragma once

#include <thread>

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
            std::filesystem::path get_fixed_path(const std::filesystem::path& file_path) const;

            b8 exists(const std::filesystem::path& path) const;
            b8 is_directory(const std::filesystem::path& path) const;
    };

    class FileWatcher
    {
        public:
            FileWatcher();
            ~FileWatcher();

            void watch_file(const std::filesystem::path& file_path);
            void stop_watching_file(const std::filesystem::path& file_path);
            void reset_file_status(const std::filesystem::path& file_path);

            b8 was_file_modified(const std::filesystem::path& file_path);

        private:
            struct FileStatus
            {
                    b8 modified = false;
                    std::filesystem::file_time_type last_write_time;
            };

            std::thread watcher_thread;
            std::map<str, FileStatus> files_on_watch;
            std::mutex files_mutex;
            b8 running;
    };
};  // namespace mag
