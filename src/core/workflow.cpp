//
// Created by hohaia on 13/09/2026.
//

#include <algorithm>
#include <cctype>

#include "workflow.h"

namespace ict
{
    //log in, fetch the controllers settings, and build the tables for the sub menus.
    LoginResult loginAndFetchSettings(ControllerApi& wx, const std::string& userName, const std::string& password)
    {
        auto passwordHash = ControllerApi::sha1Hex(password);
        std::ranges::transform(passwordHash, passwordHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        if (!wx.login(userName, passwordHash))
        {
            return {false, std::nullopt};
        }
        return {true, wx.sendRequest("Detail", "GXT_CONTROLLERSETTINGS_TBL")};
    }

    //build a sub menu.
    std::vector<DynamicMenuItem> buildSubMenu(ControllerApi& wx, const std::string& listName)
    {
        std::vector<DynamicMenuItem> items;
        std::optional<ResponseTable> list = wx.sendRequest("List", listName);
        if (!list)
        {
            return items;
        }
        int displayKey = 1;
        for (const auto& [recId, label] : *list)
        {
            items.push_back(DynamicMenuItem(displayKey++, label, recId));
        }
        items.push_back(DynamicMenuItem(0, "Back", ""));

        return items;
    }

    //lookup a sub menu, build it and add it to cache if it does not exist
    const std::vector<DynamicMenuItem>& getSubMenu(ControllerApi& wx, SubMenuCache& cache, const std::string& listName)
    {
        auto it = cache.find(listName);
        if (it == cache.end())
        {
            it = cache.emplace(listName, buildSubMenu(wx, listName)).first;
        }
        return it->second;
    }
}
