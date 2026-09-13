#include <exception>
#include <iostream>
#include <string>

#include "console.h"
#include "ControllerAPI.h"
#include "workflow.h"

int main()
{
    try
    {
        const std::string domain = ict::readLine("\nIP/Domain: ");
        const std::string userName = ict::readLine("\nUsername: ");
        const std::string password = ict::readPassword("\nPassword: ");
        const bool isHttps = ict::readYesNo("\nHttps? (y/n): ");

        ict::ControllerAPI wx(domain, isHttps);
        ict::LoginResult wxLogin = ict::loginAndFetchSettings(wx, userName, password);
        if (!wxLogin.loggedIn)
        {
            std::cout << "\nFailed to log in: " << wx.lastError() << std::endl;
            return 1;
        }
        if (wxLogin.settings)
        {
            std::cout << "\nLogged in... Getting controller settings." << std::endl;
            ict::printTable(*wxLogin.settings);
        }
        else
        {
            std::cout << "\nCould not get controller settings: " << wx.lastError() << std::endl;
        }

        // the session is closed by ControllerAPI's destructor when wx goes out of scope
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
