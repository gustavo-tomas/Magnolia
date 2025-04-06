#include "project.hpp"

namespace sprout
{
    Project::Project() = default;
    Project::~Project() = default;

    void Project::set_name(const str& name) { this->name = name; }
    void Project::set_asset_dir(const str& asset_dir) { this->asset_dir = asset_dir; }
    void Project::set_relative_start_scene_path(const str& start_scene_path)
    {
        this->start_scene_path = start_scene_path;
    }

    const fs::path& Project::get_name() const { return name; }
    const fs::path& Project::get_asset_dir() const { return asset_dir; }
    const fs::path& Project::get_relative_start_scene_path() const { return start_scene_path; }

    namespace project
    {
        b8 load(const str& file_path, Project& project)
        {
            fs::json project_data;
            if (!fs::read_json_data(file_path, project_data))
            {
                LOG_ERROR("Failed to read project file: '{0}'", file_path);
                return false;
            }

            if (!project_data.contains("AssetDirectory") || !project_data.contains("StartScene") ||
                !project_data.contains("Name"))
            {
                LOG_ERROR("Project file '{0}' has missing fields", file_path);
                return false;
            }

            const str project_name = project_data["Name"].get<str>();
            const str project_asset_dir = project_data["AssetDirectory"].get<str>();
            const str start_scene_file_path = project_data["StartScene"].get<str>();

            project.set_name(project_name);
            project.set_asset_dir(project_asset_dir);
            project.set_relative_start_scene_path(start_scene_file_path);

            return true;
        }

        b8 save(const str& file_path, Project& project)
        {
            fs::json data;
            data["Name"] = project.get_name();
            data["AssetDirectory"] = project.get_asset_dir();
            data["StartScene"] = project.get_relative_start_scene_path();

            if (!fs::write_json_data(file_path, data))
            {
                LOG_ERROR("Failed to write project to file: '{0}'", file_path);
                return false;
            }

            return true;
        }
    };  // namespace project
};      // namespace sprout
