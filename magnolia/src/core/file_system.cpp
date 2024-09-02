#include "core/file_system.hpp"

#include <fstream>

#include "core/logger.hpp"

namespace mag
{
    b8 FileSystem::read_binary_data(const std::filesystem::path& file_path, Buffer& buffer)
    {
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

        file.read(buffer.cast<char>(), size);
        file.close();

        return true;
    }

    b8 FileSystem::write_binary_data(const std::filesystem::path& file_path, Buffer& buffer)
    {
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

        file.write(buffer.cast<char>(), buffer.get_size());
        file.close();

        return true;
    }

    b8 FileSystem::read_json_data(const std::filesystem::path& file_path, json& data)
    {
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
};  // namespace mag
