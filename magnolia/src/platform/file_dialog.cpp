#include "platform/file_dialog.hpp"

#include "core/logger.hpp"
#include "portable-file-dialogs/portable-file-dialogs.h"

namespace mag
{
    FileDialog::FileDialog()
    {
#if defined(MAG_DEBUG)
        pfd::settings::verbose(true);
        pfd::settings::rescan();
#endif

        // Check that a backend is available
        if (!pfd::settings::available())
        {
            LOG_ERROR("Portable File Dialogs are not available on this platform.");
            return;
        }
    }

    str FileDialog::open_file(const str& title, const std::filesystem::path& directory,
                              const std::vector<str>& filters) const
    {
        auto f = pfd::open_file(title, directory, filters, pfd::opt::multiselect);

        const auto result = f.result();
        if (result.empty())
        {
            return {};
        }

        return result.front();
    }

    str FileDialog::save_file(const str& title, const std::filesystem::path& file_path,
                              const std::vector<str>& filters) const
    {
        auto f = pfd::save_file(title, file_path, filters);

        const auto result = f.result();
        return result;
    }

    void FileDialog::notify(const str& title, const str& message, const DialogIcon icon)
    {
        pfd::notify(title, message, static_cast<pfd::icon>(icon));
    }

    DialogButton FileDialog::message(const str& title, const str& message, const DialogChoice choice,
                                     const DialogIcon icon) const
    {
        auto m = pfd::message(title, message, static_cast<pfd::choice>(choice), static_cast<pfd::icon>(icon));

        return static_cast<DialogButton>(m.result());
    }

    str FileDialog::select_folder(const str& title, const std::filesystem::path& path) const
    {
        auto dir = pfd::select_folder(title, path);

        const auto result = dir.result();
        return result;
    }
};  // namespace mag
