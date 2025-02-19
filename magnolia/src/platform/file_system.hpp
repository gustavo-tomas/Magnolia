#pragma once

#include <thread>

#include "core/types.hpp"
#include "nlohmann/json.hpp"

// @TODO: figure out how to handle file paths (see shader.cpp)

namespace mag
{
    using json = nlohmann::ordered_json;

    struct Buffer;

    namespace fs
    {
        b8 read_binary_data(const std::filesystem::path& file_path, Buffer& buffer);
        b8 write_binary_data(const std::filesystem::path& file_path, Buffer& buffer);

        b8 read_json_data(const std::filesystem::path& file_path, json& data);
        b8 write_json_data(const std::filesystem::path& file_path, json& data);

        b8 create_directories(const std::filesystem::path& path);

        str get_file_extension(const std::filesystem::path& file_path);
        std::filesystem::path get_fixed_path(const std::filesystem::path& file_path);

        b8 exists(const std::filesystem::path& path);
        b8 is_directory(const std::filesystem::path& path);
    };  // namespace fs

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
