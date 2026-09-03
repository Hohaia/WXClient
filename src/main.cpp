#include <algorithm>
#include <cctype>
#include <iostream>

#include "ControllerAPI.h"

int main()
{
    std::string domain;
    char httpsInput;
    bool isHttps = false;
    std::string user;
    std::string pass;

    std::cout << "\nIP/Domain: ";
    std::cin >> domain;
    std::cout << "\nHttps? (y/n): ";
    std::cin >> httpsInput;
    if (httpsInput == 'y' || httpsInput == 'Y')
    {
        isHttps = true;
    }
    else if (httpsInput == 'n' || httpsInput == 'N')
    {
        isHttps = false;
    }
    else
    {
        std::cout << "\nInvalid input. Exiting Programme.";
        return(0);
    }
    std::cout << "\nUsername: ";
    std::cin >> user;
    std::cout << "\nPassword: ";
    std::cin >> pass;

    try
    {
        ict::ControllerAPI wx(domain, isHttps);
        auto pswHash = ict::ControllerAPI::sha1FromString(pass);
        std::ranges::transform(pswHash, pswHash.begin(),
                               [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        if (!wx.login(user, pswHash))
        {
            std::cout << "\nFailed to log in." << std::endl;
            return 1;
        }
        std::cout << "\nLogged in." << std::endl;

        // TODO: controller queries go here, e.g.
        //   const auto settings = wx.getControllerSettings();
        //   std::cout << "Serial: " << settings.at("SERIALNUMBER") << std::endl;

        // the session is closed by ControllerAPI's destructor when wx goes out of scope
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}