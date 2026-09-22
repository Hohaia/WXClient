//
// Created by hohaia on 14/09/2026.
//

#ifndef WXCLIENT_MENU_TABLES_H
#define WXCLIENT_MENU_TABLES_H

#include <array>
#include <string>
#include <string_view>

namespace ict
{
    struct MenuItem
    {
        std::string recId;
        std::string label;
    };

    struct MenuTable
    {
        std::string_view label;
        std::string_view listName;
    };

    constexpr std::array MenuTables{
        MenuTable{"Access Levels", "GXT_ACCESSLEVELS_TBL"},
        MenuTable{"Analog Expanders", "GXT_ANALOGEXPANDERS_TBL"},
        MenuTable{"Areas", "GXT_AREAS_TBL"},
        MenuTable{"Area Groups", "GXT_AREAGROUPS_TBL"},
        MenuTable{"Automation", "GXT_AUTOMATION_TBL"},
        MenuTable{"Cameras", "GXT_CAMERAS_TBL"},
        MenuTable{"Controllers", "GXT_CONTROLLERS_TBL"},
        MenuTable{"Controller Settings", "GXT_CONTROLLERSETTINGS_TBL"},
        MenuTable{"Credential Types", "GXT_CREDENTIALTYPES_TBL"},
        MenuTable{"Data Values", "GXT_DATAVALUES_TBL"},
        MenuTable{"Daylight Savings", "GXT_DAYLIGHTSAVINGS_TBL"},
        MenuTable{"Doors", "GXT_DOORS_TBL"},
        MenuTable{"Door Groups", "GXT_DOORGROUPS_TBL"},
        MenuTable{"Door Types", "GXT_DOORTYPES_TBL"},
        MenuTable{"Elevator Cars", "GXT_ELEVATORCARS_TBL"},
        MenuTable{"Elevator Groups", "GXT_ELEVATORGROUPS_TBL"},
        MenuTable{"Event Reports", "GXT_EVENTREPORTS_TBL"},
        MenuTable{"Floor", "GXT_FLOORS_TBL"},
        MenuTable{"Floor Groups", "GXT_FLOORGROUPS_TBL"},
        MenuTable{"Holiday Group", "GXT_HOLIDAYGROUPS_TBL"},
        MenuTable{"Inputs", "GXT_INPUTS_TBL"},
        MenuTable{"Input Expanders", "GXT_INPUTEXPANDERS_TBL"},
        MenuTable{"Input Types", "GXT_INPUTTYPES_TBL"},
        MenuTable{"Keypads", "GXT_KEYPADS_TBL"},
        MenuTable{"Keypad Groups", "GXT_KEYPADGROUPS_TBL"},
        MenuTable{"Menu Groups", "GXT_MENUGROUPS_TBL"},
        MenuTable{"Outputs", "GXT_PGMS_TBL"},
        MenuTable{"Output Expanders", "GXT_PGMEXPANDERS_TBL"},
        MenuTable{"Output Groups", "GXT_PGMGROUPS_TBL"},
        MenuTable{"Phone Numbers", "GXT_PHONENUMBERS_TBL"},
        MenuTable{"Programmable Functions", "GXT_PROGRAMMABLEFUNCTIONS_TBL"},
        MenuTable{"Reader Expanders", "GXT_READEREXPANDERS_TBL"},
        MenuTable{"Schedules", "GXT_SCHEDULES_TBL"},
        MenuTable{"Services", "GXT_SERVICES_TBL"},
        MenuTable{"Site Details", "GXT_SITEDETAILS_TBL"},
        MenuTable{"Reader Units", "GXT_READERUNITS_TBL"},
        MenuTable{"Trouble Inputs", "GXT_TROUBLEINPUTS_TBL"},
        MenuTable{"Users", "GXT_USERS_TBL"}
    };
}

#endif //WXCLIENT_MENU_TABLES_H
