//
// Created by hohaia on 13/09/2026.
//

#include <algorithm>
#include <cctype>

#include "workflow.h"

namespace ict
{
    LoginResult loginAndFetchSettings(Controller_Api& wx, const std::string& userName, const std::string& password)
    {
        auto passwordHash = Controller_Api::sha1Hex(password);
        std::ranges::transform(passwordHash, passwordHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if (!wx.login(userName, passwordHash))
        {
            return {false, std::nullopt};
        }
        return {true, wx.sendRequest("Detail", "GXT_CONTROLLERSETTINGS_TBL")};
    }
}
