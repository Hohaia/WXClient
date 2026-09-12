#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <string>

#include "console.h"
#include "ControllerAPI.h"

int main()
{
    try
    {
        const std::string domain = ict::readLine("\nIP/Domain: ");
        const std::string user = ict::readLine("\nUsername: ");
        const std::string pass = ict::readPassword("\nPassword: ");
        const bool isHttps = ict::readYesNo("\nHttps? (y/n): ");

        ict::ControllerAPI wx(domain, isHttps);
        auto pswHash = ict::ControllerAPI::sha1Hex(pass);
        std::ranges::transform(pswHash, pswHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        if (!wx.login(user, pswHash))
        {
            std::cout << "\nFailed to log in." << std::endl;
            return 1;
        }
        std::cout << "\nLogged in... Getting controller settings." << std::endl;
        const auto settings = wx.request("Detail", "GXT_CONTROLLERSETTINGS_TBL" );
        ict::printTable(settings);

        // the session is closed by ControllerAPI's destructor when wx goes out of scope
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}