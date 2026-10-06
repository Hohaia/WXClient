//
// Created by hohaia on 14/09/2026.
//

#include "session.h"

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "console.h"
#include "control.h"
#include "controller_api.h"
#include "file_transfer.h"
#include "helpers.h"
#include "logger.h"
#include "table_names.h"
#include "status.h"
#include "workflow.h"

namespace ict::cli
{
    namespace
    {
        // The action to take for a specific item in the main menu.
        enum class MainMenuAction {OpenSubMenu, EventLogs, Backup, RestartModules, Restart};

        // How a submenu was exited.
        enum class MenuResult {Back, Logout};

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
            MainMenuItem{"Event Logs",          MainMenuAction::EventLogs,      ""},
            MainMenuItem{"Backup",              MainMenuAction::Backup,         ""},
            MainMenuItem{"Restart Modules",     MainMenuAction::RestartModules, ""},
            MainMenuItem{"Restart Controller",  MainMenuAction::Restart,        ""}
        };

        // Build a menu label with the item's live status, e.g. "Front Door  [Locked - Left Open]" (empty fields hidden).
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

        // Add a time to a date-only entry ("01-10-2026" -> "01-10-2026T00:00:00"); anything else is passed through.
        std::string withTime(const std::string& date, const std::string& time)
        {
            if (date.size() != 10)
                return date;
            return date + "T" + time;
        }

        // Show an untrusted certificate's fingerprint and ask whether to trust it.
        bool confirmCertificate(const std::string& fingerprint, const bool changed)
        {
            if (changed)
                std::cout << "\nWARNING: the controller's certificate has CHANGED since you last trusted it.\n"
                          << "A renewal or factory reset does this, but so would someone intercepting the connection.\n";
            else
                std::cout << "\nThe controller's certificate is not trusted yet.\n";
            std::cout << "SHA-256 fingerprint: " << core::formatFingerprint(fingerprint) << "\n"
                      << "Only accept it if it matches the controller's certificate.\n";
            return readYesNo("\nTrust this certificate? (y/n): ");
        }

        // Run a control menu for the selected record.
        MenuResult cliCommandMenu(core::ControllerApi& wx, const core::TableInfo& table, const core::RecordEntry& record)
        {
            // Build the commands menu and handle a table with no commands.
            const auto commands = core::findControlCommands(table.name);
            const auto tableName = std::string(table.name);
            if (commands.empty())
            {
                printError("No control commands found: " + tableName);
                waitForEnter();
                return MenuResult::Back;
            }
            std::vector<MenuItem> display;
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
                const std::string choice = printMenu(display, title, wx.serialNumber());
                if (choice == "r")
                    continue; // While loop will refresh statuses.
                if (choice == "0")
                    return MenuResult::Back;
                if (choice == "exit")
                    return MenuResult::Logout;
                const auto& command = commands[std::stoi(choice) - 1];

                // Send a command via core (Control, name, RecId, Command).
                core::KeyValueList params{{"RecId", record.recId}, {"Command", std::to_string(command.code)}};
                if (!command.data1.empty())
                    params.emplace_back("Data1", readLine(std::string(command.data1) + ": "));
                if (!command.data2.empty())
                    params.emplace_back("Data2", readLine(std::string(command.data2) + ": "));
                if (!wx.sendCommand(core::CommandType::Control, tableName, params))
                {
                    // On failure, printError with wx.lastError().
                    printError("Command failed: " + wx.lastError());
                    waitForEnter();
                }
            }
        }

        // Run a sub menu.
        MenuResult cliSubMenu(core::ControllerApi& wx, core::RecordListCache& cachedRecordLists, const core::TableInfo& subMenu)
        {
            const auto* fetched = core::getRecordList(wx, cachedRecordLists, std::string(subMenu.name));
            if (!fetched)
            {
                printError("Could not load " + std::string(subMenu.label) + ": " + wx.lastError());
                waitForEnter();
                return MenuResult::Back;
            }
            const auto& items = *fetched;
            const bool hasCommands = !core::findControlCommands(subMenu.name).empty();
            while (true)
            {
                const auto statuses = core::fetchStatuses(wx, std::string(subMenu.name));
                std::vector<std::string> labels;
                labels.reserve(items.size());
                for (const auto& item : items)
                    labels.emplace_back(itemLabel(item, statuses, subMenu.name));

                std::vector<MenuItem> display;
                for (size_t i = 0; i < items.size(); ++i)
                    display.emplace_back(hasCommands ? std::to_string(i + 1) : "", labels[i]);
                display.emplace_back("r", "Refresh Status", true);
                display.emplace_back("0", "Back");

                const std::string choice = printMenu(display, subMenu.label, wx.serialNumber());
                if (choice == "r")
                    continue; // While loop will refresh the statuses.
                if (choice == "0")
                    return MenuResult::Back;
                if (choice == "exit")
                    return MenuResult::Logout;
                if (cliCommandMenu(wx, subMenu, items[std::stoi(choice) - 1]) == MenuResult::Logout)
                    return MenuResult::Logout;
            }
        }

        // Download the event log as a .csv to the Downloads folder.
        void cliDownloadEventLog(core::ControllerApi& wx)
        {
            const auto directory = core::defaultDownloadDirectory();
            if (!directory)
            {
                printError("Download failed: could not find the Downloads folder.");
                waitForEnter();
                return;
            }
            std::cout << "\nDates are dd-mm-yyyy (or dd-mm-yyyyTHH:MM:SS), leave blank for no limit.\n";
            const std::string startDate = withTime(readLine("Start date: "), "00:00:00");
            const std::string endDate = withTime(readLine("End date: "), "23:59:59");
            std::cout << "\nDownloading events...\n";
            const core::DownloadResult result = core::saveEventLog(wx, *directory, startDate, endDate);
            if (result.isEmpty)
            {
                // Not an error: nothing was logged, so don't point at the log file.
                std::cout << "\n" << result.error << "\n";
                waitForEnter();
                return;
            }
            if (!result.path)
            {
                printError("Download failed: " + result.error);
                waitForEnter();
                return;
            }
            std::cout << "\nEvents saved to " << result.path->string() << " (" << result.bytes << " bytes)\n";
            waitForEnter();
        }

        // Run the event log viewer, 20 events at a time.
        MenuResult cliEventLog(core::ControllerApi& wx)
        {
            auto request = core::EventRequest::Latest;
            while (true)
            {
                const auto events = core::fetchEvents(wx, request);
                if (!events)
                {
                    printError("Could not load events: " + wx.lastError());
                    waitForEnter();
                    return MenuResult::Back;
                }
                std::vector<MenuItem> display;
                for (const auto& event : *events)
                    display.emplace_back("", event);
                if (events->empty())
                    display.emplace_back("", "No events");
                display.emplace_back("1", "Previous 20", true);
                display.emplace_back("2", "Next 20");
                display.emplace_back("r", "Latest");
                display.emplace_back("d", "Download .csv");
                display.emplace_back("0", "Back");

                const std::string choice = printMenu(display, "Event Log", wx.serialNumber());
                if (choice == "d")
                {
                    cliDownloadEventLog(wx);
                    request = core::EventRequest::Latest;
                    continue;
                }
                if (choice == "0")
                    return MenuResult::Back;
                if (choice == "exit")
                    return MenuResult::Logout;
                if (choice == "1")
                    request = core::EventRequest::Previous;
                else if (choice == "2")
                    request = core::EventRequest::Next;
                else
                    request = core::EventRequest::Latest;   // "r"
            }
        }

        // Download a backup to the Downloads folder.
        void cliBackup(core::ControllerApi& wx)
        {
            const auto directory = core::defaultDownloadDirectory();
            if (!directory)
            {
                printError("Backup failed: could not find the Downloads folder.");
                waitForEnter();
                return;
            }
            std::cout << "\nDownloading backup...\n";
            const core::DownloadResult result = core::saveBackup(wx, *directory);
            if (!result.path)
            {
                printError("Backup failed: " + result.error);
                waitForEnter();
                return;
            }
            std::cout << "\nBackup saved to " << result.path->string() << " (" << result.bytes << " bytes)\n";
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
            std::vector<MenuItem> display;
            int key = 1;
            for (const auto& item : mainMenu)
                display.emplace_back(std::to_string(key++), item.label);
            while (true)
            {
                const std::string choice = printMenu(display, "Main Menu", wx.serialNumber());
                if (choice == "exit")
                    return 0;
                try
                {
                    // Keys are 1..N in 'mainMenu' order, so the key is the index + 1.
                    switch (const auto& selected = mainMenu[std::stoi(choice) - 1]; selected.action)
                    {
                        case MainMenuAction::OpenSubMenu:
                            if (cliSubMenu(wx, cachedRecordLists, {selected.label, selected.tableName}) == MenuResult::Logout)
                                return 0;
                            break;
                        case MainMenuAction::EventLogs:
                            if (cliEventLog(wx) == MenuResult::Logout)
                                return 0;
                            break;
                        case MainMenuAction::Backup:
                            cliBackup(wx);
                            break;
                        case MainMenuAction::RestartModules:
                            cliRestartModules(wx);
                            break;
                        case MainMenuAction::Restart:
                            if (!cliRestart(wx))
                                break;
                            return 0;
                    }
                }
                catch (const std::exception& e)
                {
                    // Network errors (timeouts, dropped connections) end the session cleanly instead of crashing.
                    core::logMessage(core::LogLevel::Error, "cliMainMenu", e.what());
                    printError(e.what());
                    std::cout << "\nClosing the session.\n";
                    waitForEnter();
                    return 1;
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
        auto result = core::loginAndFetchSettings(wx, userName, password);
        if (result == core::LoginResult::UntrustedCertificate || result == core::LoginResult::CertificateChanged)
        {
            if (!confirmCertificate(wx.untrustedFingerprint(), result == core::LoginResult::CertificateChanged))
            {
                std::cout << "\nCertificate not trusted; not logging in.\n";
                return 1;
            }
            if (!core::trustCertificate(wx))
                printError("Could not save the certificate; you'll be asked again next login.");
            result = core::loginAndFetchSettings(wx, userName, password);
        }
        if (result != core::LoginResult::LoggedIn)
        {
            printError("Failed to log in: " + wx.lastError());
            return 1;
        }
        std::cout << "\nLogged in...\n";
        if (!wx.settings())
        {
            printError("Could not get controller settings: " + wx.lastError());
            return 1;
        }
        // NOTE: this is the main body of the programme.
        return cliMainMenu(wx);
    }
}
