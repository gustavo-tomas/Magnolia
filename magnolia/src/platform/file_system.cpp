#include "magnolia/platform/file_system.hpp"

#include <algorithm>
#include <fstream>
#include <mutex>
#include <thread>

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/json.hpp"

namespace mag
{
    namespace fs
    {
        struct FileStatus
        {
                b8 modified = false;
                std::filesystem::file_time_type last_write_time;
        };

        struct FileWatcher
        {
                std::thread watcher_thread;
                std::unordered_map<str, FileStatus> files_on_watch;
                std::mutex files_mutex;
                b8 running = false;
        };

        struct State
        {
                FileWatcher fw;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            // Initialize file watcher
            state->fw.running = true;

            state->fw.watcher_thread = std::thread([]
            {
                while (state->fw.running)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));

                    std::vector<str> marked_for_removal;

                    const std::scoped_lock<std::mutex> lock(state->fw.files_mutex);
                    for (auto& [file_path, file_status] : state->fw.files_on_watch)
                    {
                        // Remove files that have been deleted
                        if (!fs::exists(file_path))
                        {
                            marked_for_removal.push_back(file_path);
                            continue;
                        }

                        auto current_write_time = std::filesystem::last_write_time(file_path);
                        if (current_write_time != file_status.last_write_time)
                        {
                            file_status.last_write_time = current_write_time;
                            file_status.modified = true;
                        }
                    }

                    for (const auto& file_path : marked_for_removal)
                    {
                        state->fw.files_on_watch.erase(file_path);
                    }
                }
            });

            return state != nullptr;
        }

        void shutdown()
        {
            state->fw.running = false;

            if (state->fw.watcher_thread.joinable())
            {
                state->fw.watcher_thread.join();
            }

            delete state;
        }

        b8 read_binary_data(const fs::path& raw_file_path, Buffer& buffer)
        {
            const fs::path file_path = get_fixed_path(raw_file_path);

            std::ifstream file(file_path, std::ios::binary | std::ios::ate);

            // Failed to open the file
            if (!file)
            {
                LOG_ERROR("Failed to open file: '{0}'", file_path.string());
                return false;
            }

            const std::streampos end = file.tellg();
            file.seekg(0, std::ios::beg);
            const i64 size = end - file.tellg();

            // File is empty
            if (size <= 0)
            {
                LOG_ERROR("File is empty: '{0}'", file_path.string());
                return false;
            }

            buffer.data.resize(size);

            file.read(buffer.cast<c8>(), size);
            file.close();

            return true;
        }

        b8 write_binary_data(const fs::path& raw_file_path, Buffer& buffer)
        {
            const fs::path file_path = get_fixed_path(raw_file_path);

            std::ofstream file(file_path, std::ios::binary);

            if (!file)
            {
                LOG_ERROR("Failed to open file: '{0}'", file_path.string());
                return false;
            }

            if (buffer.get_size() == 0)
            {
                LOG_ERROR("Buffer is empty");
                return false;
            }

            file.write(buffer.cast<c8>(), static_cast<i64>(buffer.get_size()));
            file.close();

            return true;
        }

        b8 read_json_data(const fs::path& raw_file_path, fs::json& data)
        {
            const fs::path file_path = get_fixed_path(raw_file_path);

            // Parse data from the json file
            std::ifstream file(file_path);

            if (!file)
            {
                LOG_ERROR("Failed to open file: '{0}'", file_path.string());
                return false;
            }

            data = fs::json::parse(file, nullptr, false);

            if (data.is_discarded())
            {
                LOG_ERROR("Invalid json data: '{0}'", file_path.string());
                return false;
            }

            return true;
        }

        b8 write_json_data(const fs::path& raw_file_path, const fs::json& data)
        {
            const fs::path file_path = get_fixed_path(raw_file_path);

            std::ofstream file(file_path);

            if (!file)
            {
                LOG_ERROR("Failed to open file: '{0}'", file_path.string());
                return false;
            }

            file << std::setw(2) << data;
            file.close();

            return true;
        }

        b8 create_directories(const fs::path& raw_file_path)
        {
            const fs::path path = get_fixed_path(raw_file_path);

            if (fs::exists(path))
            {
                return true;
            }

            return std::filesystem::create_directories(path);
        }

        fs::path get_fixed_path(const fs::path& file_path)
        {
            str fixed_path = file_path.string();

            // Replace backslashes
            std::ranges::replace_if(fixed_path.begin(), fixed_path.end(), [](const auto& ch) { return ch == '\\'; },
                                    '/');

            return fixed_path;
        }

        str get_file_extension(const fs::path& raw_file_path)
        {
            const fs::path file_path = get_fixed_path(raw_file_path);
            return file_path.extension().c_str();
        }

        str get_file_name(const fs::path& raw_file_path)
        {
            const fs::path file_path = get_fixed_path(raw_file_path);
            return file_path.stem().string();
        }

        b8 exists(const fs::path& raw_file_path)
        {
            const fs::path path = get_fixed_path(raw_file_path);
            return std::filesystem::exists(path);
        }

        b8 is_directory(const fs::path& raw_file_path)
        {
            const fs::path path = get_fixed_path(raw_file_path);
            return std::filesystem::is_directory(path);
        }

        void watch_file(const fs::path& file_path)
        {
            const std::scoped_lock<std::mutex> lock(state->fw.files_mutex);
            if (!fs::exists(file_path) || state->fw.files_on_watch.contains(file_path))
            {
                return;
            }

            state->fw.files_on_watch[file_path].last_write_time = std::filesystem::last_write_time(file_path);
            state->fw.files_on_watch[file_path].modified = false;
        }

        void stop_watching_file(const fs::path& file_path)
        {
            const std::scoped_lock<std::mutex> lock(state->fw.files_mutex);
            if (state->fw.files_on_watch.contains(file_path))
            {
                state->fw.files_on_watch.erase(file_path);
            }
        }

        void reset_file_status(const fs::path& file_path)
        {
            std::unique_lock<std::mutex> lock(state->fw.files_mutex);
            if (state->fw.files_on_watch.contains(file_path))
            {
                lock.unlock();

                stop_watching_file(file_path);
                watch_file(file_path);
            }
        }

        b8 was_file_modified(const fs::path& file_path)
        {
            const std::scoped_lock<std::mutex> lock(state->fw.files_mutex);
            return state->fw.files_on_watch.contains(file_path) && state->fw.files_on_watch[file_path].modified;
        }
    };  // namespace fs
};  // namespace mag
