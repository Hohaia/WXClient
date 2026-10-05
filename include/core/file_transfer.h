//
// Created by hohaia on 05/10/2026.
//

#ifndef WXCLIENT_FILE_TRANSFER_H
#define WXCLIENT_FILE_TRANSFER_H

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

#include "controller_api.h"

namespace ict::core
{
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
}

#endif //WXCLIENT_FILE_TRANSFER_H
