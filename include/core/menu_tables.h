//
// Created by hohaia on 14/09/2026.
//

#ifndef WXCLIENT_MENU_TABLES_H
#define WXCLIENT_MENU_TABLES_H

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace ict
{
    struct StaticMenuItem
    {
        const int key;
        const std::string_view label;
    };

    struct DynamicMenuItem
    {
        int key;
        std::string label;
        std::string recId;
    };

    constexpr std::array mainMenu{
        StaticMenuItem{1, "Doors"},
        StaticMenuItem{2, "Areas"},
        StaticMenuItem{3, "Outputs"},
        StaticMenuItem{4, "Inputs"},
        StaticMenuItem{5, "Trouble Inputs"},
        StaticMenuItem{0, "Logout"}
    };

    const std::vector<std::string> subMenus{
        "GXT_ACCESSLEVELS_TBL",
        "GXT_ANALOGEXPANDERS_TBL",
        "GXT_AREAS_TBL",
        "GXT_AREAGROUPS_TBL",
        "GXT_AUTOMATION_TBL",
        "GXT_CAMERAS_TBL",
        "GXT_CONTROLLERS_TBL",
        "GXT_CONTROLLERSETTINGS_TBL",
        "GXT_CREDENTIALTYPES_TBL",
        "GXT_DATAVALUES_TBL",
        "GXT_DAYLIGHTSAVINGS_TBL",
        "GXT_DOORS_TBL",
        "GXT_DOORGROUPS_TBL",
        "GXT_DOORTYPES_TBL",
        "GXT_ELEVATORCARS_TBL",
        "GXT_ELEVATORGROUPS_TBL",
        "GXT_EVENTREPORTS_TBL",
        "GXT_FLOORS_TBL",
        "GXT_FLOORGROUPS_TBL",
        "GXT_HOLIDAYGROUPS_TBL",
        "GXT_INPUTS_TBL",
        "GXT_INPUTEXPANDERS_TBL",
        "GXT_INPUTTYPES_TBL",
        "GXT_KEYPADS_TBL",
        "GXT_KEYPADGROUPS_TBL",
        "GXT_MENUGROUPS_TBL",
        "GXT_PGMS_TBL",
        "GXT_PGMEXPANDERS_TBL",
        "GXT_PGMGROUPS_TBL",
        "GXT_PHONENUMBERS_TBL",
        "GXT_PROGRAMMABLEFUNCTIONS_TBL",
        "GXT_READEREXPANDERS_TBL",
        "GXT_SCHEDULES_TBL",
        "GXT_SERVICES_TBL",
        "GXT_SITEDETAILS_TBL",
        "GXT_READERUNITS_TBL",
        "GXT_TROUBLEINPUTS_TBL",
        "GXT_USERS_TBL"
    };
}

#endif //WXCLIENT_MENU_TABLES_H
