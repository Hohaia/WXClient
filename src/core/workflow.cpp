//
// Created by hohaia on 13/09/2026.
//

#include "workflow.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "controller_api.h"
#include "helpers.h"
#include "logger.h"

namespace ict
{
    namespace
    {
        // Fetch a table's record list, or nullopt if the List request failed.
        std::optional<std::vector<RecordEntry>> fetchRecordList(ControllerApi& wx, const std::string& tableName)
        {
            const std::optional<KeyValueList> list = wx.sendRequest(RequestType::List, tableName);
            if (!list)
                return std::nullopt;   // Already logged by ControllerApi.
            std::vector<RecordEntry> items;
            for (const auto& [recId, label] : *list)
                items.emplace_back(recId, label);
            return items;
        }

        // Build the backup file name: Db_<serial>_<dd_Mon_yyyy>.bak
        std::string backupFileName(const std::string& serialNumber)
        {
            const std::time_t now = std::time(nullptr);
            std::tm tm{};
            localtime_r(&now, &tm);
            std::ostringstream date;
            date << std::put_time(&tm, "%d_%b_%Y");
            return "Db_" + serialNumber + "_" + date.str() + ".bak";
        }
    }

    // Find the default folder to save backups in.
    std::optional<std::filesystem::path> defaultBackupDirectory()
    {
        const char* home = std::getenv("HOME");
        if (!home)
        {
            logMessage(LogLevel::Error, "defaultBackupDirectory", "HOME environment variable is not set");
            return std::nullopt;
        }
        return std::filesystem::path(home) / "Downloads";
    }

    // Download a backup from the controller and save it in 'directory'.
    BackupResult saveBackup(ControllerApi& wx, const std::filesystem::path& directory)
    {
        const auto backup = wx.downloadBackup();
        if (!backup)
            return {std::nullopt, 0, wx.lastError()};

        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec)
        {
            const std::string error = "Could not create " + directory.string() + ": " + ec.message();
            logMessage(LogLevel::Error, "saveBackup", error);
            return {std::nullopt, 0, error};
        }

        const std::filesystem::path filePath = directory / backupFileName(wx.m_serialNumber);
        std::ofstream out(filePath, std::ios::binary);
        out.write(backup->data(), static_cast<std::streamsize>(backup->size()));
        if (!out)
        {
            const std::string error = "Could not write " + filePath.string();
            logMessage(LogLevel::Error, "saveBackup", error);
            return {std::nullopt, 0, error};
        }
        return {filePath, backup->size(), ""};
    }

    // Log in, fetch the controllers settings (returns 'true' on successful login).
    bool loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password)
    {
        auto passwordHash = sha1Hex(password);
        std::ranges::transform(passwordHash, passwordHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if (!wx.login(userName, passwordHash))
        {
            return false;
        }
        wx.m_settings = wx.sendRequest(RequestType::Detail, "GXT_CONTROLLERSETTINGS_TBL");
        if (!wx.m_settings)
            return true;   // Logged in, but no settings; runCli checks wx.m_settings.
        const auto serialIt = std::ranges::find_if(*wx.m_settings,
                              [](const auto& kv) { return kv.first == "SERIALNUMBER"; });
        wx.m_serialNumber = serialIt != wx.m_settings->end() ? serialIt->second : "UNKNOWN";
        return true;
    }

    // Look up a record list, fetching and caching it on first use; nullptr if the fetch failed.
    const std::vector<RecordEntry>* getRecordList(ControllerApi& wx, RecordListCache& cache, const std::string& tableName)
    {
        if (const auto it = cache.find(tableName); it != cache.end())
            return &it->second;
        auto items = fetchRecordList(wx, tableName);
        if (!items)
            return nullptr;   // Not cached, so the next visit retries.
        return &cache.emplace(tableName, std::move(*items)).first->second;
    }

    // Fetch the live statuses for a list, keyed by RecId; nullopt if the request failed.
    std::optional<StatusMap> fetchStatuses(ControllerApi& wx, const std::string& tableName)
    {
        const std::optional<KeyValueList> response = wx.sendRequest(RequestType::Status, tableName);
        if (!response)
            return std::nullopt;   // Already logged by ControllerApi.
        StatusMap statuses;
        for (const auto& [key, value] : *response)
        {
            const auto digits = key.find_first_of("0123456789");
            if (digits == std::string::npos)
                continue;
            statuses.emplace(key.substr(digits), value);   // "Door12" -> "12"
        }
        return statuses;
    }
}
