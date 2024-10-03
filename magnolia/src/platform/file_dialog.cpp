#include "platform/file_dialog.hpp"

#include <filesystem>

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

    str FileDialog::open_file(const str& title, const std::vector<str>& filters) const
    {
        auto f = pfd::open_file(title, std::filesystem::current_path(), filters, pfd::opt::multiselect);

        const auto result = f.result();
        if (result.empty())
        {
            return {};
        }

        return result.front();
    }

    str FileDialog::save_file(const str& title, const str& file_name, const std::vector<str>& filters) const
    {
        auto f = pfd::save_file(title, std::filesystem::current_path() / file_name, filters);

        const auto result = f.result();
        return result;
    }

    // @TODO;
    void FileDialog::notify()
    {
        // Notification
        pfd::notify("Important Notification", "This is ' a message, pay \" attention \\ to it!", pfd::icon::info);
    }

    // @TODO;
    void FileDialog::message()
    {
        // Message box with nice message
        auto m =
            pfd::message("Personal Message", "You are an amazing person, don't let anyone make you think otherwise.",
                         pfd::choice::yes_no_cancel, pfd::icon::warning);

        // Optional: do something while waiting for user action
        for (int i = 0; i < 10 && !m.ready(1000); ++i) std::cout << "Waited 1 second for user input...\n";

        // Do something according to the selected button
        switch (m.result())
        {
            case pfd::button::yes:
                std::cout << "User agreed.\n";
                break;

            case pfd::button::no:
                std::cout << "User disagreed.\n";
                break;

            case pfd::button::cancel:
                std::cout << "User freaked out.\n";
                break;

            default:
                break;  // Should not happen
        }
    }

    // @TODO;
    void FileDialog::select_folder()
    {
        // Directory selection
        auto dir = pfd::select_folder("Select any directory", pfd::path::home()).result();
        std::cout << "Selected dir: " << dir << "\n";
    }
};  // namespace mag