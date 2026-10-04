//
// Created by hohaia on 13/09/2026.
//

#ifndef WXCLIENT_WORKFLOW_H
#define WXCLIENT_WORKFLOW_H

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "controller_api.h"
#include "table_names.h"

namespace ict::core
{
    // Cache of a table's contents ("GXT_DOORS_TBL": {"0", "Entry Door"}, {"1", "Side Door"}, {...}).
    using RecordListCache = std::unordered_map<std::string, std::vector<RecordEntry>>;

    // recID -> raw status, (e.g. "0" -> "23,3,0").
    using StatusMap = std::unordered_map<std::string, std::string>;

    // The result of a controller file download.
    struct DownloadResult
    {
        std::optional<std::filesystem::path> path;  // Set on success.
        std::size_t bytes{};
        std::string error;                          // Set on failure.
        bool isEmpty{};                             // Event log only: header with no events.
    };

    std::optional<std::filesystem::path> defaultDownloadDirectory();
    DownloadResult saveBackup(ControllerApi& wx, const std::filesystem::path& directory);
    DownloadResult saveEventLog(ControllerApi& wx, const std::filesystem::path& directory,
                                const std::string& startDate, const std::string& endDate);
    bool loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password);
    const std::vector<RecordEntry>* getRecordList(ControllerApi& wx, RecordListCache& cache, const std::string& tableName);
    std::optional<StatusMap> fetchStatuses(ControllerApi& wx, const std::string& tableName);
    std::optional<std::vector<std::string>> fetchEvents(ControllerApi& wx, EventRequest request);
}

#endif //WXCLIENT_WORKFLOW_H
