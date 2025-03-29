#include "platform/file_dialog.hpp"

#include <filesystem>

#include "core/logger.hpp"
#include "portable-file-dialogs/portable-file-dialogs.h"

namespace mag
{
    b8 FileDialog::initialized = false;

    b8 FileDialog::initialize()
    {
#if MAG_CONFIG_DEBUG
        pfd::settings::verbose(true);
#endif

        // Check that a backend is available
        if (!pfd::settings::available())
        {
            LOG_ERROR("Portable File Dialogs are not available on this platform.");
            return false;
        }

        initialized = true;
        return true;
    }

    void FileDialog::shutdown() { initialized = false; }

    str FileDialog::open_file(const str& title, const std::vector<str>& filters)
    {
        if (!initialized) return "";

        auto f = pfd::open_file(title, std::filesystem::current_path(), filters, pfd::opt::multiselect);

        const auto result = f.result();
        if (result.empty())
        {
            return {};
        }

        return result.front();
    }

    str FileDialog::save_file(const str& title, const str& file_name, const std::vector<str>& filters)
    {
        if (!initialized) return "";

        auto f = pfd::save_file(title, std::filesystem::current_path() / file_name, filters);

        const auto result = f.result();
        return result;
    }

    void FileDialog::notify(const str& title, const str& message, const DialogIcon icon)
    {
        if (!initialized) return;

        pfd::notify(title, message, static_cast<pfd::icon>(icon));
    }

    DialogButton FileDialog::message(const str& title, const str& message, const DialogChoice choice,
                                     const DialogIcon icon)
    {
        if (!initialized) return DialogButton::Abort;

        auto m = pfd::message(title, message, static_cast<pfd::choice>(choice), static_cast<pfd::icon>(icon));

        return static_cast<DialogButton>(m.result());
    }

    str FileDialog::select_folder(const str& title)
    {
        if (!initialized) return "";

        auto dir = pfd::select_folder(title, std::filesystem::current_path());

        const auto result = dir.result();
        return result;
    }
};  // namespace mag
