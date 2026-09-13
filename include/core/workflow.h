//
// Created by hohaia on 13/09/2026.
//

#ifndef WXCLIENT_WORKFLOW_H
#define WXCLIENT_WORKFLOW_H

#include <optional>
#include <string>

#include "ControllerAPI.h"
#include "helpers.h"

namespace ict
{
    struct LoginResult
    {
        bool loggedIn;
        std::optional<ResponseTable> settings;
    };

    // Log in, then fetch the controller settings table if login succeeded.
    // On either failure, check wx.lastError() for why.
    LoginResult loginAndFetchSettings(ControllerAPI& wx, const std::string& userName, const std::string& password);
}

#endif //WXCLIENT_WORKFLOW_H
