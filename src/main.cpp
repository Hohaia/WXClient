#include <algorithm>
#include <cctype>
#include <iostream>

#include "console.h"
#include "ControllerAPI.h"

int main()
{
    try
    {
        const std::string domain = readLine("\nIP/Domain: ");
        const bool isHttps = readYesNo("\nHttps? (y/n): ");
        const std::string user = readLine("\nUsername: ");
        const std::string pass = readPassword("\nPassword: ");

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