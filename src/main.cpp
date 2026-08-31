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
    const std::string pswHash = ict::ControllerAPI::sha1FromString(pass);

    wx.login(user, pswHash);

    return(0);
}