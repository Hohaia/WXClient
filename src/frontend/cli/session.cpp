//
// Created by hohaia on 14/09/2026.
//

#include <iostream>
#include <string>

#include "session.h"
#include "console.h"
#include "Controller_Api.h"
#include "static_menu_tables.h"
#include "workflow.h"

namespace ict
{
    int runCli()
    {
        const std::string domain = readLine("\nIP/Domain: ");
        const std::string userName = readLine("\nUsername: ");
        const std::string password = readPassword("\nPassword: ");
        const bool isHttps = readYesNo("\nHttps? (y/n): ");

        Controller_Api wx(domain, isHttps); //the session is closed by Controller_Api's destructor when wx goes out of scope
        LoginResult wxLogin = loginAndFetchSettings(wx, userName, password);
        if (!wxLogin.loggedIn)
        {
            std::cout << "\nFailed to log in: " << wx.lastError() << std::endl;
            return 1;
        }
        std::cout << "\nLogged in... Getting controller settings." << std::endl;
        if (wxLogin.settings)
        {
            printTable(*wxLogin.settings);
            //TODO make "List" requests to build dynamic tables (or include this on loginAndFetchSettings())
            while (true) //loop until "Logout" is selected from mainMenu
            {
                switch (printMenu(mainMenu))
                {
                    case 0: return 0;
                    case 1: break; //TODO
                    case 2: break; //TODO
                    case 3: break; //TODO
                    case 4: break; //TODO
                    case 5: break; //TODO
                }
            }
        }
        else
        {
            std::cout << "\nCould not get controller settings: " << wx.lastError() << std::endl;
        }
        return 0;
    }
}
