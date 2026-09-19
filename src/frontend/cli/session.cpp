//
// Created by hohaia on 14/09/2026.
//

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "session.h"
#include "console.h"
#include "controller_api.h"
#include "menu_tables.h"
#include "workflow.h"

namespace ict
{
    namespace
    {
        //run a sub menu
        void cliSubMenu(ControllerApi& wx, SubMenuCache& subMenus, const std::string& listName)
        {
            const auto& items = getSubMenu(wx, subMenus, listName);
            while (true) //loop until "Back" is selected
            {
                switch (printMenu<DynamicMenuItem>(items))
                {
                    case 1: //TODO
                    break;
                }
                break; //TODO remove placeholder line
            }
        }

        //run the main menu
        int cliMainMenu(ControllerApi& wx)
        {
            SubMenuCache subMenus;
            while (true) //loop until "Logout" is selected from mainMenu
            {
                std::string listName;
                switch (printMenu<StaticMenuItem>(mainMenu))
                {
                    case 1: //1. Doors
                        listName = "GXT_DOORS_TBL";
                        cliSubMenu(wx, subMenus, listName);
                        break;
                    case 2: //2. Areas
                        listName = "GXT_AREAS_TBL";
                        cliSubMenu(wx, subMenus, listName);
                        break;
                    case 3: //3. Outputs
                        listName = "GXT_PGMS_TBL";
                        cliSubMenu(wx, subMenus, listName);
                        break;
                    case 4: //4. Inputs
                        listName = "GXT_INPUTS_TBL";
                        cliSubMenu(wx, subMenus, listName);
                        break;
                    case 5: //5. Trouble Inputs
                        listName = "GXT_TROUBLEINPUTS_TBL";
                        cliSubMenu(wx, subMenus, listName);
                        break;
                    case 0: //0. Logout
                        return 0;
                }
            }
        }
    }

    //run the cli front end interface
    int runCli()
    {
        const std::string domain = readLine("\nIP/Domain: ");
        const std::string userName = readLine("\nUsername: ");
        const std::string password = readPassword("\nPassword: ");
        const bool isHttps = readYesNo("\nHttps? (y/n): ");

        ControllerApi wx(domain, isHttps); //the session is closed by ControllerApi's destructor when wx goes out of scope
        LoginResult wxLogin = loginAndFetchSettings(wx, userName, password);
        if (!wxLogin.loggedIn)
        {
            std::cout << "\nFailed to log in: " << wx.lastError() << std::endl;
            return 1;
        }
        std::cout << "\nLogged in... Getting controller settings." << std::endl;
        if (wxLogin.settings)
        {
            //NOTE: this is the main body of the programme.
            printTable(*wxLogin.settings);
            return cliMainMenu(wx);
        }
        else
        {
            std::cout << "\nCould not get controller settings: " << wx.lastError() << std::endl;
        }
        return 0;
    }
}
