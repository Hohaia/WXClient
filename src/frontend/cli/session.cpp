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
#include "control.h"
#include "controller_api.h"
#include "table_names.h"
#include "status.h"
#include "workflow.h"

namespace ict::cli
{
    namespace
    {
        // The action to take for a specific item in the main menu.
        enum class MainMenuAction {OpenSubMenu, Backup, RestartModules, Restart, Logout};

        // The contents of a menu item ("1. Doors" -> label: "Doors", action: OpenSubMenu, name: "GXT_DOORS_TBL").
        struct MainMenuItem
        {
            std::string_view label;
            MainMenuAction action;
            std::string_view tableName;   // Only meaningful when action == OpenSubMenu.
        };

        // An array of each item in the main menu and what to do when it is selected.
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

        // Build a menu label with the item's live status, e.g. "Front Door  [Locked, Closed, [No Flags]]".
        std::string itemLabel(const core::RecordEntry& item, const std::optional<core::StatusMap>& statuses, std::string_view tableName)
        {
            if (!statuses)
                return item.label;
            const auto it = statuses->find(item.recId);
            if (it == statuses->end())
                return item.label;
            std::string text;
            for (const auto& field : core::decodeStatus(tableName, it->second))
            {
                if (field.isEmpty)
                    continue;
                if (!text.empty())
                    text += " - ";
                text += field.text;
            }
            return item.label + "  [" + text + "]";
        }

        // Run a control menu for the selected record.
        void cliCommandMenu(core::ControllerApi& wx, const core::TableInfo& table, const core::RecordEntry& record)
        {
            // Build the commands menu and handle a table with no commands.
            const auto commands = core::findControlCommands(table.name);
            const auto tableName = std::string(table.name);
            if (commands.empty())
            {
                printError("No control commands found: " + std::string(tableName));
                waitForEnter();
                return;
            }
            std::vector<StaticMenuItem> display;
            for (size_t i = 0; i < commands.size(); ++i)
                display.emplace_back(std::to_string(i + 1), commands[i].label);
            display.emplace_back("r", "Refresh Status", true);
            display.emplace_back("0", "Back");
            // A while loop will handle status refresh on command send.
            while (true)
            {
                // Display live status of the selected record in the menu title.
                const auto statuses = core::fetchStatuses(wx, tableName);
                const std::string title = itemLabel(record, statuses, tableName);

                // Show this table's commands and read the choice (0 = back).
                const std::string choice = printMenu<StaticMenuItem>(wx, display, title);
                if (choice == "r")
                    continue; // While loop will refresh statuses.
                if (choice == "0")
                    break;
                const auto& command = commands[std::stoi(choice) - 1];

                // Send a command via core (Control, name, RecId, Command).
                if (!wx.sendCommand(core::CommandType::Control, tableName,
                                 {{"RecId", record.recId}, {"Command", std::to_string(command.code)}}))
                {
                    // On failure, printError with wx.lastError().
                    printError("Command failed: " + wx.lastError());
                    waitForEnter();
                }
            }
        }

        // Run a sub menu.
        void cliSubMenu(core::ControllerApi& wx, core::RecordListCache& cachedRecordLists, const core::TableInfo& subMenu)
        {
            const auto* fetched = core::getRecordList(wx, cachedRecordLists, std::string(subMenu.name));
            if (!fetched)
            {
                printError("Could not load " + std::string(subMenu.label) + ": " + wx.lastError());
                waitForEnter();
                return;
            }
            const auto& items = *fetched;

            while (true)
            {
                const auto statuses = core::fetchStatuses(wx, std::string(subMenu.name));
                std::vector<std::string> labels;
                labels.reserve(items.size());
                for (const auto& item : items)
                    labels.emplace_back(itemLabel(item, statuses, subMenu.name));

                std::vector<StaticMenuItem> display;
                for (size_t i = 0; i < items.size(); ++i)
                    display.emplace_back(std::to_string (i + 1), labels[i]);
                display.emplace_back("r", "Refresh Status", true);
                display.emplace_back("0", "Back");

                const std::string choice = printMenu<StaticMenuItem>(wx, display, subMenu.label);
                if (choice == "r")
                    continue; // While loop will refresh the statuses.
                if (choice == "0")
                    break;
                cliCommandMenu(wx, subMenu, items[std::stoi(choice) - 1]);
            }
        }

        // Download a backup to the Downloads folder.
        void cliBackup(core::ControllerApi& wx)
        {
            const auto directory = core::defaultBackupDirectory();
            if (!directory)
            {
                printError("Backup failed: could not find the Downloads folder.");
                waitForEnter();
                return;
            }
            std::cout << "\nDownloading backup...\n";
            const core::BackupResult result = core::saveBackup(wx, *directory);
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
        void cliRestartModules(core::ControllerApi& wx)
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
        bool cliRestart(core::ControllerApi& wx)
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
        int cliMainMenu(core::ControllerApi& wx)
        {
            core::RecordListCache cachedRecordLists;
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

        core::ControllerApi wx(domain, isHttps); // The session is closed by ControllerApi's destructor when wx goes out of scope.
        if (!core::loginAndFetchSettings(wx, userName, password))
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
