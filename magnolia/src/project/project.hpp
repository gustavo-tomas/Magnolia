#pragma once

#include <filesystem>
#include <memory>

#include "core/assert.hpp"
#include "core/types.hpp"

namespace mag
{
    struct ProjectConfig
    {
            str name = "Untitled";

            std::filesystem::path start_scene;
            std::filesystem::path asset_directory;
    };

    class Project
    {
        public:
            static const std::filesystem::path& get_project_directory()
            {
                ASSERT(active_project, "No active project");
                return active_project->project_directory;
            }

            static std::filesystem::path get_asset_directory()
            {
                ASSERT(active_project, "No active project");
                return get_project_directory() / active_project->config.asset_directory;
            }

            static std::shared_ptr<Project> get_active() { return active_project; };

            ProjectConfig& get_config() { return config; };

            static std::shared_ptr<Project> new_project();
            static std::shared_ptr<Project> load_project(const std::filesystem::path& path);
            static void save_active_project(const std::filesystem::path& path);

        private:
            ProjectConfig config;
            std::filesystem::path project_directory;

            inline static std::shared_ptr<Project> active_project;
    };
};  // namespace mag
