#pragma once

#include <thread>

#include "core/types.hpp"
#include "nlohmann/json.hpp"

// @TODO: figure out how to handle file paths (see shader.cpp)

namespace mag
{
    struct Buffer;

    namespace fs
    {
        using path = std::filesystem::path;
        using json = nlohmann::ordered_json;

        b8 read_binary_data(const fs::path& file_path, Buffer& buffer);
        b8 write_binary_data(const fs::path& file_path, Buffer& buffer);

        b8 read_json_data(const fs::path& file_path, fs::json& data);
        b8 write_json_data(const fs::path& file_path, fs::json& data);

        b8 create_directories(const fs::path& path);

        str get_file_extension(const fs::path& file_path);
        fs::path get_fixed_path(const fs::path& file_path);

        b8 exists(const fs::path& path);
        b8 is_directory(const fs::path& path);
    };  // namespace fs

    class FileWatcher
    {
        public:
            FileWatcher();
            ~FileWatcher();

            void watch_file(const fs::path& file_path);
            void stop_watching_file(const fs::path& file_path);
            void reset_file_status(const fs::path& file_path);

            b8 was_file_modified(const fs::path& file_path);

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
