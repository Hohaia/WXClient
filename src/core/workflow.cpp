//
// Created by hohaia on 13/09/2026.
//

#include "workflow.h"

#include <algorithm>
#include <cctype>

namespace ict
{
    // Log in, fetch the controllers settings, and build the tables for the sub menus.
    LoginResult loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password)
    {
        auto passwordHash = ControllerApi::sha1Hex(password);
        std::ranges::transform(passwordHash, passwordHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if (!wx.login(userName, passwordHash))
        {
            return {false, std::nullopt};
        }
        wx.m_settings = wx.sendRequest("Detail", "GXT_CONTROLLERSETTINGS_TBL");
        return {true, wx.m_settings};
    }

    // Build a sub menu.
    std::vector<MenuItem> buildSubMenu(ControllerApi& wx, const std::string& listName)
    {
        std::vector<MenuItem> items;
        std::optional<ResponseTable> list = wx.sendRequest("List", listName);
        if (!list)
        {
            return items;
        }
        for (const auto& [recId, label] : *list)
        {
            items.emplace_back(recId, label);
        }
        return items;
    }

    // Lookup a sub menu, build it and add it to cache if it does not exist.
    const std::vector<MenuItem>& getSubMenu(ControllerApi& wx, SubMenuCache& cache, const std::string& listName)
    {
        auto it = cache.find(listName);
        if (it == cache.end())
        {
            it = cache.emplace(listName, buildSubMenu(wx, listName)).first;
        }
        return it->second;
    }
}
