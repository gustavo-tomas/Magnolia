#pragma once

#include <vector>

#include "core/types.hpp"

namespace mag
{
    class FileDialog
    {
        public:
            FileDialog();

            str open_file(const str& title, const std::vector<str>& filters = {"All Files", "*"}) const;
            str save_file(const str& title, const str& file_name,
                          const std::vector<str>& filters = {"All Files", "*"}) const;

            void notify();
            void message();
            void select_folder();
    };
};  // namespace mag
