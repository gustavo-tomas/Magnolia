#pragma once

#include "magnolia/platform/file_system.hpp"

namespace mag
{
    class MAG_API Project
    {
        public:
            Project();
            ~Project();

            void set_name(const str& name);
            void set_asset_dir(const str& asset_dir);
            void set_relative_start_scene_path(const str& start_scene_path);

            const fs::path& get_name() const;
            const fs::path& get_asset_dir() const;
            const fs::path& get_relative_start_scene_path() const;

        private:
            fs::path name = "";
            fs::path asset_dir = "";
            fs::path start_scene_path = "";
    };

    namespace project
    {
        MAG_API b8 load(const str& file_path, Project& project);
        MAG_API b8 save(const str& file_path, Project& project);
    };  // namespace project
};  // namespace mag
