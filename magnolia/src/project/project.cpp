#include "project/project.hpp"

#include "project/project_serializer.hpp"

namespace mag
{
    std::shared_ptr<Project> Project::new_project()
    {
        active_project = std::make_shared<Project>();
        return active_project;
    }

    std::shared_ptr<Project> Project::load_project(const std::filesystem::path& path)
    {
        active_project = std::make_shared<Project>();
        ProjectSerializer project_serializer(active_project);
        project_serializer.deserialize(path);

        return active_project;
    }

    void Project::save_active_project(const std::filesystem::path& path)
    {
        ProjectSerializer project_serializer(active_project);
        project_serializer.serialize(path);
    }
};  // namespace mag
