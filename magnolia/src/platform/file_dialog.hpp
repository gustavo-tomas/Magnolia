#pragma once

#include <filesystem>
#include <vector>

#include "core/types.hpp"

namespace mag
{
    enum class DialogIcon
    {
        Info = 0,
        Warning,
        Error,
        Question
    };

    enum class DialogChoice
    {
        Ok = 0,
        OkCancel,
        YesNo,
        YesNoCancel,
        RetryCancel,
        AbortRetryIgnore
    };

    enum class DialogButton
    {
        Cancel = -1,
        Ok,
        Yes,
        No,
        Abort,
        Retry,
        Ignore
    };

    class FileDialog
    {
        public:
            FileDialog();

            str open_file(const str& title, const std::filesystem::path& directory,
                          const std::vector<str>& filters = {"All Files", "*"}) const;
            str save_file(const str& title, const std::filesystem::path& file_path,
                          const std::vector<str>& filters = {"All Files", "*"}) const;

            void notify(const str& title, const str& message, const DialogIcon icon);
            DialogButton message(const str& title, const str& message, const DialogChoice choice,
                                 const DialogIcon icon) const;
            str select_folder(const str& title, const std::filesystem::path& path) const;
    };
};  // namespace mag
