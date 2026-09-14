//
// Created by hohaia on 14/09/2026.
//

#ifndef WXCLIENT_STATIC_MENU_TABLES_H
#define WXCLIENT_STATIC_MENU_TABLES_H

#include <array>
#include <string_view>

namespace ict
{
    struct MenuItem
    {
        int key;
        std::string_view label;
    };

    constexpr std::array mainMenu{
        MenuItem{1, "Doors"},
        MenuItem{2, "Areas"},
        MenuItem{3, "Outputs"},
        MenuItem{4, "Inputs"},
        MenuItem{5, "Trouble Inputs"},
        MenuItem{0, "Logout"}
    };
}

#endif //WXCLIENT_STATIC_MENU_TABLES_H
