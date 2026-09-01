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

    ict::ControllerAPI wx(domain, isHttps);
    auto pswHash = ict::ControllerAPI::sha1FromString(pass);
    std::ranges::transform(pswHash, pswHash.begin(),
                           [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    wx.login(user, pswHash);
    wx.logout();

    return(0);
}