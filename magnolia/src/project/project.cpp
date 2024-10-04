#include "project/project.hpp"

#include "core/application.hpp"

namespace mag
{
    void serialize(Project& project, const std::filesystem::path& path);
    void deserialize(Project& project, const std::filesystem::path& path);

    std::shared_ptr<Project> Project::new_project()
    {
        active_project = std::make_shared<Project>();
        return active_project;
    }

    std::shared_ptr<Project> Project::load_project(const std::filesystem::path& path)
    {
        active_project = std::make_shared<Project>();
        deserialize(*active_project, path);

        active_project->project_directory = path.parent_path();

        return active_project;
    }

    void Project::save_active_project(const std::filesystem::path& path)
    {
        serialize(*active_project, path);

        active_project->project_directory = path.parent_path();
    }

    void serialize(Project& project, const std::filesystem::path& file_path)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const auto& config = project.get_config();

        json data;
        data["Type"] = "Project";
        data["Name"] = config.name;
        data["StartScene"] = config.start_scene.string();
        data["AssetDirectory"] = config.asset_directory.string();

        file_system.write_json_data(file_path, data);
    }

    void deserialize(Project& project, const std::filesystem::path& file_path)
    {
        LOG_INFO("Deserializing project '{0}'", file_path.string());

        auto& app = get_application();
        auto& file_system = app.get_file_system();

        json data;
        if (file_system.read_json_data(file_path, data))
        {
            auto& config = project.get_config();

            config.name = data["Name"].get<str>();
            config.start_scene = data["StartScene"].get<str>();
            config.asset_directory = data["AssetDirectory"].get<str>();
        }
    }
};  // namespace mag
