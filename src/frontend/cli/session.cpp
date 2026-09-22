//
// Created by hohaia on 14/09/2026.
//

#include "session.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "console.h"
#include "controller_api.h"
#include "menu_tables.h"
#include "workflow.h"

namespace ict
{
    namespace
    {
        enum class MainMenuAction { OpenSubMenu, Backup, Restart, Logout };

        struct MainMenuItem
        {
            std::string_view label;
            MainMenuAction action;
            std::string_view listName;   // Only meaningful when action == OpenSubMenu.
        };

        constexpr std::array mainMenu{
            MainMenuItem{"Doors",           MainMenuAction::OpenSubMenu, "GXT_DOORS_TBL"},
            MainMenuItem{"Areas",           MainMenuAction::OpenSubMenu, "GXT_AREAS_TBL"},
            MainMenuItem{"Outputs",         MainMenuAction::OpenSubMenu, "GXT_PGMS_TBL"},
            MainMenuItem{"Inputs",          MainMenuAction::OpenSubMenu, "GXT_INPUTS_TBL"},
            MainMenuItem{"Trouble Inputs",  MainMenuAction::OpenSubMenu, "GXT_TROUBLEINPUTS_TBL"},
            MainMenuItem{"Backup",          MainMenuAction::Backup,      ""},
            MainMenuItem{"Restart",         MainMenuAction::Restart,     ""},
            MainMenuItem{"Logout",          MainMenuAction::Logout,      ""}
        };

        // Run a control menu using the selected RecId.
       void cliCommandMenu(ControllerApi& wx, SubMenuCache& cachedSubMenus, const std::string& recId)
        {
            //TODO
        }

        // Run a sub menu.
        void cliSubMenu(ControllerApi& wx, SubMenuCache& cachedSubMenus, const MenuTable& subMenu)
       {
           const auto& items = getSubMenu(wx, cachedSubMenus, std::string(subMenu.listName));

           std::vector<StaticMenuItem> display;
           for (size_t i = 0; i < items.size(); ++i)
               display.emplace_back(static_cast<int>(i) + 1, items[i].label);
           display.emplace_back(0, "Back");

           while (true)
           {
               const int choice = printMenu<StaticMenuItem>(display, subMenu.label);
               if (choice == 0)
                   break;
               cliCommandMenu(wx, cachedSubMenus, items[choice - 1].recId);
           }
       }

        // Run the main menu.
        int cliMainMenu(ControllerApi& wx)
       {
           SubMenuCache cachedSubMenus;
           std::vector<StaticMenuItem> display;
           int key = 1;
           for (const auto& item : mainMenu)
               display.emplace_back(item.action == MainMenuAction::Logout ? 0 : key++, item.label);

           while (true)
           {
               const int choice = printMenu<StaticMenuItem>(display, "Main Menu");
               const auto displayIt = std::ranges::find_if(display,
                                      [choice](const StaticMenuItem& item) { return item.key == choice; });
               const auto& selected = mainMenu[std::distance(display.begin(), displayIt)];

               switch (selected.action)
               {
                   case MainMenuAction::OpenSubMenu:
                   {
                       const auto it = std::ranges::find_if(MenuTables,
                           [&](const MenuTable& s) { return s.listName == selected.listName; });
                       cliSubMenu(wx, cachedSubMenus, *it);
                       break;
                   }
                   case MainMenuAction::Backup:
                   {
                       std::cout << "\nDownloading backup...\n";
                       const auto backup = wx.downloadBackup();
                       if (!backup)
                       {
                           std::cout << "\nBackup failed: " << wx.lastError() << "\n";
                           break;
                       }
                       const auto serialIt = std::ranges::find_if(*wx.m_settings,
                                             [](const auto& kv) { return kv.first == "SERIALNUMBER"; });
                       const std::string serial = serialIt != wx.m_settings->end() ? serialIt->second : "UNKNOWN";

                       std::ostringstream dateStream;
                       const std::time_t now = std::time(nullptr);
                       dateStream << std::put_time(std::localtime(&now), "%d_%b_%Y");

                       const std::filesystem::path downloadsDir = std::filesystem::path(std::getenv("HOME")) / "Downloads";
                       std::filesystem::create_directories(downloadsDir);
                       const std::filesystem::path filePath = downloadsDir / ("Db_" + serial + "_" + dateStream.str() + ".bak");

                       std::ofstream out(filePath, std::ios::binary);
                       out.write(backup->data(), static_cast<std::streamsize>(backup->size()));

                       std::cout << "\nBackup saved to " << filePath << " (" << backup->size() << " bytes)\n";
                       break;
                   }
                   case MainMenuAction::Restart:
                       if (readYesNo("\nAre you sure you want to restart the Controller? (y/n): "))
                           wx.sendCommand("Command", "RestartController");
                       break;
                   case MainMenuAction::Logout:
                       return 0;
               }
           }
       }
    }

    // Run the cli front end interface.
    int runCli()
    {
        const std::string domain = readLine("\nIP/Domain: ");
        const std::string userName = readLine("\nUsername: ");
        const std::string password = readPassword("\nPassword: ");
        const bool isHttps = readYesNo("\nIs the controller using Https? (y/n): ");

        ControllerApi wx(domain, isHttps); // The session is closed by ControllerApi's destructor when wx goes out of scope.
        LoginResult wxLogin = loginAndFetchSettings(wx, userName, password);
        if (!wxLogin.loggedIn)
        {
            std::cout << "\nFailed to log in: " << wx.lastError() << std::endl;
            return 1;
        }
        std::cout << "\nLogged in... Getting controller settings." << std::endl;
        if (wxLogin.settings)
        {
            // NOTE: this is the main body of the programme.
            printTable(*wxLogin.settings);
            readLine("\nPress [Enter] to continue...\n");
            return cliMainMenu(wx);
        }
        else
        {
            std::cout << "\nCould not get controller settings: " << wx.lastError() << std::endl;
        }
        return 0;
    }
}
