#include "core/file_system.hpp"

#include <algorithm>
#include <fstream>

#include "core/logger.hpp"

namespace mag
{
    b8 FileSystem::read_binary_data(const std::filesystem::path& raw_file_path, Buffer& buffer) const
    {
        const auto file_path = get_fixed_path(raw_file_path);

        std::ifstream file(file_path, std::ios::binary | std::ios::ate);

        // Failed to open the file
        if (!file)
        {
            LOG_ERROR("Failed to open file: '{0}'", file_path.string());
            return false;
        }

        std::streampos end = file.tellg();
        file.seekg(0, std::ios::beg);
        const u64 size = end - file.tellg();

        // File is empty
        if (size == 0)
        {
            LOG_ERROR("File is empty: '{0}'", file_path.string());
            return false;
        }

        buffer.data.resize(size);

        file.read(buffer.cast<c8>(), size);
        file.close();

        return true;
    }

    b8 FileSystem::write_binary_data(const std::filesystem::path& raw_file_path, Buffer& buffer) const
    {
        const auto file_path = get_fixed_path(raw_file_path);

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

        file.write(buffer.cast<c8>(), buffer.get_size());
        file.close();

        return true;
    }

    b8 FileSystem::read_json_data(const std::filesystem::path& raw_file_path, json& data) const
    {
        const auto file_path = get_fixed_path(raw_file_path);

        // Parse data from the json file
        std::ifstream file(file_path);

        if (!file)
        {
            LOG_ERROR("Failed to open file: '{0}'", file_path.string());
            return false;
        }

        data = json::parse(file, nullptr, false);

        if (data.is_discarded())
        {
            LOG_ERROR("Invalid json data: '{0}'", file_path.string());
            return false;
        }

        return true;
    }

    b8 FileSystem::write_json_data(const std::filesystem::path& raw_file_path, json& data) const
    {
        const auto file_path = get_fixed_path(raw_file_path);

        std::ofstream file(file_path);

        if (!file)
        {
            LOG_ERROR("Failed to open file: '{0}'", file_path.string());
            return false;
        }

        file << std::setw(4) << data;
        file.close();

        return true;
    }

    b8 FileSystem::create_directories(const std::filesystem::path& raw_file_path) const
    {
        const auto path = get_fixed_path(raw_file_path);

        if (exists(path))
        {
            return true;
        }

        return std::filesystem::create_directories(path);
    }

    std::filesystem::path FileSystem::get_fixed_path(const std::filesystem::path& file_path) const
    {
        str fixed_path = file_path.string();

        // Replace backslashes
        std::replace_if(
            fixed_path.begin(), fixed_path.end(), [](const auto& ch) { return ch == '\\'; }, '/');

        return fixed_path;
    }

    str FileSystem::get_file_extension(const std::filesystem::path& raw_file_path) const
    {
        const auto file_path = get_fixed_path(raw_file_path);
        return file_path.extension().c_str();
    }

    b8 FileSystem::exists(const std::filesystem::path& raw_file_path) const
    {
        const auto path = get_fixed_path(raw_file_path);
        return std::filesystem::exists(path);
    }

    b8 FileSystem::is_directory(const std::filesystem::path& raw_file_path) const
    {
        const auto path = get_fixed_path(raw_file_path);
        return std::filesystem::is_directory(path);
    }
};  // namespace mag
