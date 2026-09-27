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
#include "table_names.h"
#include "status.h"
#include "workflow.h"

namespace ict
{
    namespace
    {
        enum class MainMenuAction { OpenSubMenu, Backup, RestartModules, Restart, Logout };

        struct MainMenuItem
        {
            std::string_view label;
            MainMenuAction action;
            std::string_view tableName;   // Only meaningful when action == OpenSubMenu.
        };

        constexpr std::array mainMenu{
            MainMenuItem{"Doors",               MainMenuAction::OpenSubMenu,    "GXT_DOORS_TBL"},
            MainMenuItem{"Areas",               MainMenuAction::OpenSubMenu,    "GXT_AREAS_TBL"},
            MainMenuItem{"Outputs",             MainMenuAction::OpenSubMenu,    "GXT_PGMS_TBL"},
            MainMenuItem{"Inputs",              MainMenuAction::OpenSubMenu,    "GXT_INPUTS_TBL"},
            MainMenuItem{"Trouble Inputs",      MainMenuAction::OpenSubMenu,    "GXT_TROUBLEINPUTS_TBL"},
            MainMenuItem{"Backup",              MainMenuAction::Backup,         ""},
            MainMenuItem{"Restart Modules",     MainMenuAction::RestartModules, ""},
            MainMenuItem{"Restart Controller",  MainMenuAction::Restart,        ""},
            MainMenuItem{"Logout",              MainMenuAction::Logout,         ""}
        };

        // Run a control menu using the selected RecId.
        void cliCommandMenu(ControllerApi& wx, const TableInfo& table, const std::string& recId)
        {
            //TODO
        }

        // Build a menu label with the item's live status, e.g. "Front Door  [Locked, Closed, None]".
        std::string itemLabel(const RecordEntry& item, const std::optional<StatusMap>& statuses, std::string_view tableName)
        {
            if (!statuses)
                return item.label;
            const auto it = statuses->find(item.recId);
            if (it == statuses->end())
                return item.label;
            std::string text;
            for (const auto& field : decodeStatus(tableName, it->second))
            {
                if (!text.empty())
                    text += " - ";
                text += field.text;
            }
            return item.label + "  [" + text + "]";
        }

        // Run a sub menu.
        void cliSubMenu(ControllerApi& wx, RecordListCache& cachedRecordLists, const TableInfo& subMenu)
        {
            const auto* fetched = getRecordList(wx, cachedRecordLists, std::string(subMenu.tableName));
            if (!fetched)
            {
                printError("Could not load " + std::string(subMenu.label) + ": " + wx.lastError());
                waitForEnter();
                return;
            }
            const auto& items = *fetched;

            while (true)
            {
                const auto statuses = fetchStatuses(wx, std::string(subMenu.tableName));
                std::vector<std::string> labels;
                for (const auto& item : items)
                    labels.push_back(itemLabel(item, statuses, subMenu.tableName));

                std::vector<StaticMenuItem> display;
                for (size_t i = 0; i < items.size(); ++i)
                    display.emplace_back(std::to_string (i + 1), labels[i]);
                display.emplace_back("r", "Refresh Status", true);
                display.emplace_back("0", "Back");

                const std::string choice = printMenu<StaticMenuItem>(wx, display, subMenu.label);
                if (choice == "0")
                    break;
                if (choice == "r")
                    continue; // While loop will refresh the statuses.
                cliCommandMenu(wx, subMenu, items[std::stoi(choice) - 1].recId);
            }
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

        // Restart all expansion modules.
        void cliRestartModules(ControllerApi& wx)
        {
            if (!readYesNo("\nAre you sure you want to restart all modules? (y/n): "))
                return;
            if (!wx.restartAllModules())
            {
                printError("Failed to restart the modules: " + wx.lastError());
                waitForEnter();
                return;
            }
            std::cout << "\nRestarting all modules...\n";
            waitForEnter();
        }

        // Restart the controller.
        bool cliRestart(ControllerApi& wx)
        {
            if (!readYesNo("\nAre you sure you want to restart the Controller? (y/n): "))
                return false;
            if (!wx.restartController())
            {
                printError("Failed to restart the Controller: " + wx.lastError());
                waitForEnter();
                return false;
            }
            std::cout << "\nController restarting... Closing the session.\n";
            waitForEnter();
            return true;
        }

        // Run the main menu.
        int cliMainMenu(ControllerApi& wx)
        {
            RecordListCache cachedRecordLists;
            std::vector<StaticMenuItem> display;
            int key = 1;
            for (const auto& item : mainMenu)
                // Logout always gets key 0, matching the "0 = exit/back" convention
                // printMenu/cliSubMenu use elsewhere.
                display.emplace_back(item.action == MainMenuAction::Logout ? "0" : std::to_string(key++), item.label);
            while (true)
            {
                const std::string choice = printMenu<StaticMenuItem>(wx, display, "Main Menu");
                const auto displayIt = std::ranges::find_if(display, [choice](const StaticMenuItem& item)
                                                            {return item.key == choice;});
                // 'display' and 'mainMenu' are built in the same loop, same order/length,
                // so their positions line up — this breaks silently if that ever changes.
                switch (const auto& selected = mainMenu[std::distance(display.begin(), displayIt)]; selected.action)
                {
                    case MainMenuAction::OpenSubMenu:       cliSubMenu(wx, cachedRecordLists, {selected.label, selected.tableName}); break;
                    case MainMenuAction::Backup:            cliBackup(wx);  break;
                    case MainMenuAction::RestartModules:    cliRestartModules(wx); break;
                    case MainMenuAction::Restart:           if (!cliRestart(wx)) break;
                                                            return 0;
                    case MainMenuAction::Logout:            return 0;
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
