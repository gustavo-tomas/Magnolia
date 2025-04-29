#pragma once

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

    namespace fs
    {
        str open_file_dialog(const str& title, const std::vector<str>& filters = {"All Files", "*"});

        str save_file_dialog(const str& title, const str& file_name,
                             const std::vector<str>& filters = {"All Files", "*"});

        void notify_dialog(const str& title, const str& message, const DialogIcon icon);

        DialogButton message_dialog(const str& title, const str& message, const DialogChoice choice,
                                    const DialogIcon icon);

        str select_folder_dialog(const str& title);
    };  // namespace fs
};      // namespace mag
