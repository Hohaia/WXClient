//
// Created by hohaia on 14/09/2026.
//

#include "session.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
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
            const auto* fetched = getSubMenu(wx, cachedSubMenus, std::string(subMenu.listName));
            if (!fetched)
            {
                printError("Could not load " + std::string(subMenu.label) + ": " + wx.lastError());
                waitForEnter();
                return;
            }
            const auto& items = *fetched;

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

        // Open a sub menu.
        void cliOpenSubMenu(ControllerApi& wx, SubMenuCache& cachedSubMenus, const MainMenuItem& selected)
        {
            const auto it = std::ranges::find_if(MenuTables,
                            [&](const MenuTable& s) { return s.listName == selected.listName; });
            cliSubMenu(wx, cachedSubMenus, *it);
        }

        // Download a backup to the Downloads folder.
        void cliBackup(ControllerApi& wx)
        {
            const auto directory = defaultBackupDirectory();
            if (!directory)
            {
                printError("Backup failed: could not find the Downloads folder.");
                waitForEnter();
                return;
            }
            std::cout << "\nDownloading backup...\n";
            const BackupResult result = saveBackup(wx, *directory);
            if (!result.backupPath)
            {
                printError("Backup failed: " + result.error);
                waitForEnter();
                return;
            }
            std::cout << "\nBackup saved to " << result.backupPath->string() << " (" << result.bytes << " bytes)\n";
            waitForEnter();
        }

        // Restart the controller.
        void cliRestart(ControllerApi& wx)
        {
            if (!readYesNo("\nAre you sure you want to restart the Controller? (y/n): "))
                return;
            if (wx.sendCommand("RestartController"))
                return;
            printError("Failed to restart the Controller: " + wx.lastError());
            waitForEnter();
        }

        // Run the main menu.
        int cliMainMenu(ControllerApi& wx)
        {
            SubMenuCache cachedSubMenus;
            std::vector<StaticMenuItem> display;
            int key = 1;
            for (const auto& item : mainMenu)
                // Logout always gets key 0, matching the "0 = exit/back" convention
                // printMenu/cliSubMenu use elsewhere.
                display.emplace_back(item.action == MainMenuAction::Logout ? 0 : key++, item.label);

            while (true)
            {
                const int choice = printMenu<StaticMenuItem>(display, "Main Menu");
                const auto displayIt = std::ranges::find_if(display,
                                      [choice](const StaticMenuItem& item) { return item.key == choice; });

                // 'display' and 'mainMenu' are built in the same loop, same order/length,
                // so their positions line up — this breaks silently if that ever changes.
                switch (const auto& selected = mainMenu[std::distance(display.begin(), displayIt)]; selected.action)
                {
                    case MainMenuAction::OpenSubMenu: cliOpenSubMenu(wx, cachedSubMenus, selected); break;
                    case MainMenuAction::Backup:      cliBackup(wx);  break;
                    case MainMenuAction::Restart:     cliRestart(wx); break;
                    case MainMenuAction::Logout:      return 0;
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
        if (!loginAndFetchSettings(wx, userName, password))
        {
            printError("Failed to log in: " + wx.lastError());
            return 1;
        }
        std::cout << "\nLogged in...\n";
        if (!wx.m_settings)
        {
            printError("Could not get controller settings: " + wx.lastError());
            return 1;
        }
        // NOTE: this is the main body of the programme.
        printTable(*wx.m_settings);
        waitForEnter();
        return cliMainMenu(wx);
    }
}
