//
// Created by hohaia on 05/10/2026.
//

#include "file_transfer.h"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "logger.h"

namespace ict::core
{
    namespace
    {
        // Today's date for file names, e.g. "04_Oct_2026".
        std::string fileDate()
        {
            const std::time_t now = std::time(nullptr);
            std::tm tm{};
            localtime_r(&now, &tm);
            std::ostringstream date;
            date << std::put_time(&tm, "%d_%b_%Y");
            return date.str();
        }

        // Write 'data' to directory/fileName, creating the directory if needed.
        DownloadResult saveFile(const std::filesystem::path& directory, const std::string& fileName, const std::string& data)
        {
            std::error_code ec;
            std::filesystem::create_directories(directory, ec);
            if (ec)
            {
                const std::string error = "Could not create " + directory.string() + ": " + ec.message();
                logMessage(LogLevel::Error, "saveFile", error);
                return {std::nullopt, 0, error};
            }
            const std::filesystem::path filePath = directory / fileName;
            std::ofstream out(filePath, std::ios::binary);
            out.write(data.data(), static_cast<std::streamsize>(data.size()));
            if (!out)
            {
                const std::string error = "Could not write " + filePath.string();
                logMessage(LogLevel::Error, "saveFile", error);
                return {std::nullopt, 0, error};
            }
            return {filePath, data.size(), ""};
        }
    }

    // Find the default folder to save downloads in.
    std::optional<std::filesystem::path> defaultDownloadDirectory()
    {
        const char* home = std::getenv("HOME");
        if (!home)
        {
            logMessage(LogLevel::Error, "defaultDownloadDirectory", "HOME environment variable is not set");
            return std::nullopt;
        }
        return std::filesystem::path(home) / "Downloads";
    }

    // Download a backup from the controller and save it in 'directory'.
    DownloadResult saveBackup(ControllerApi& wx, const std::filesystem::path& directory)
    {
        const auto backup = wx.downloadBackup();
        if (!backup)
            return {std::nullopt, 0, wx.lastError()};
        return saveFile(directory, "Db_" + wx.serialNumber() + "_" + fileDate() + ".bak", *backup);
    }

    // Download the event log as CSV and save it in 'directory' (empty dates = no limit).
    DownloadResult saveEventLog(ControllerApi& wx, const std::filesystem::path& directory,
                                const std::string& startDate, const std::string& endDate)
    {
        const auto csv = wx.downloadEventLog(startDate, endDate);
        if (!csv)
            return {std::nullopt, 0, wx.lastError()};

        // Header only: no events in the range (or dates the controller didn't understand); don't save an empty file.
        const auto headerEnd = csv->find('\n');
        if (headerEnd == std::string::npos || csv->find_first_not_of("\r\n", headerEnd) == std::string::npos)
            return {std::nullopt, 0, "No events found for that range. Check the dates.", true};
        return saveFile(directory, "Events_" + wx.serialNumber() + "_" + fileDate() + ".csv", *csv);
    }
}