#include "project/project_serializer.hpp"

#include <fstream>

#include "nlohmann/json.hpp"

namespace mag
{
    using json = nlohmann::ordered_json;

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project) : project(project) {}

    void ProjectSerializer::serialize(const std::filesystem::path& file_path)
    {
        std::ofstream file(file_path);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to save project to file: {0}", file_path.string());
            return;
        }

        const auto& config = project->get_config();

        json data;
        data["Project"]["Name"] = config.name;
        data["Project"]["StartScene"] = config.start_scene.string();
        data["Project"]["AssetDirectory"] = config.asset_directory.string();

        file << std::setw(4) << data;
        file.close();
    }

    void ProjectSerializer::deserialize(const std::filesystem::path& file_path)
    {
        std::ifstream file(file_path);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to open project from file: {0}", file_path.string());
            return;
        }

        auto& config = project->get_config();

        const json data = json::parse(file);

        config.name = data["Project"]["Name"].get<str>();
        config.start_scene = data["Project"]["StartScene"].get<str>();
        config.asset_directory = data["Project"]["AssetDirectory"].get<str>();
    }
};  // namespace mag
