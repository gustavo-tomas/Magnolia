#pragma once

#include "project/project.hpp"

namespace mag
{
    class ProjectSerializer
    {
        public:
            ProjectSerializer(std::shared_ptr<Project> project);
            ~ProjectSerializer() = default;

            void serialize(const std::filesystem::path& file_path);
            void deserialize(const std::filesystem::path& file_path);

        private:
            std::shared_ptr<Project> project;
    };
};  // namespace mag
