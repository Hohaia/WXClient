//
// Created by hohaia on 13/09/2026.
//

#ifndef WXCLIENT_WORKFLOW_H
#define WXCLIENT_WORKFLOW_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "controller_api.h"
#include "helpers.h"
#include "menu_tables.h"

namespace ict
{
    using SubMenuCache = std::unordered_map<std::string, std::vector<DynamicMenuItem>>;

    struct LoginResult
    {
        bool loggedIn{};
        std::optional<ResponseTable> settings;
    };

    LoginResult loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password);
    std::vector<DynamicMenuItem> buildSubMenu(ControllerApi& wx, const std::string& listName);
    const std::vector<DynamicMenuItem>& getSubMenu(ControllerApi& wx, SubMenuCache& cache, const std::string& listName);
}

#endif //WXCLIENT_WORKFLOW_H
