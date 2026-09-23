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
#include "helpers.h"
#include "menu_tables.h"

namespace ict
{
    using SubMenuCache = std::unordered_map<std::string, std::vector<MenuItem>>;

    struct BackupResult
    {
        std::optional<std::filesystem::path> backupPath; // Set on success.
        std::size_t bytes{};
        std::string error;                               // Set on failure.
    };

    bool loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password);
    std::optional<std::vector<MenuItem>> buildSubMenu(ControllerApi& wx, const std::string& listName);
    const std::vector<MenuItem>* getSubMenu(ControllerApi& wx, SubMenuCache& cache, const std::string& listName);
    std::optional<std::filesystem::path> defaultBackupDirectory();
    BackupResult saveBackup(ControllerApi& wx, const std::filesystem::path& directory);
}

#endif //WXCLIENT_WORKFLOW_H
