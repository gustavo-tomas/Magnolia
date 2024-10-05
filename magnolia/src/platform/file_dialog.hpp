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

    class FileDialog
    {
        public:
            static b8 initialize();

            static void shutdown();

            static str open_file(const str& title, const std::vector<str>& filters = {"All Files", "*"});
            static str save_file(const str& title, const str& file_name,
                                 const std::vector<str>& filters = {"All Files", "*"});

            static void notify(const str& title, const str& message, const DialogIcon icon);
            static DialogButton message(const str& title, const str& message, const DialogChoice choice,
                                        const DialogIcon icon);
            static str select_folder(const str& title);
    };
};  // namespace mag
