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
        const std::string domain = readLine("\nIP/Domain: ");
        const std::string user = readLine("\nUsername: ");
        const std::string pass = readPassword("\nPassword: ");
        const bool isHttps = readYesNo("\nHttps? (y/n): ");
        const bool needsSessId = readYesNo("\nIs the controller firmware pre 4.00.1676? (y/n): ");

        ict::ControllerAPI wx(domain, isHttps, needsSessId);
        auto pswHash = ict::ControllerAPI::sha1Hex(pass);
        std::ranges::transform(pswHash, pswHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        if (!wx.login(user, pswHash))
        {
            std::cout << "\nFailed to log in." << std::endl;
            return 1;
        }
        std::cout << "\nLogged in... Getting controller settings." << std::endl;
        const auto settings = wx.fetchControllerSettings();
        printTable(settings);

        // the session is closed by ControllerAPI's destructor when wx goes out of scope
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}