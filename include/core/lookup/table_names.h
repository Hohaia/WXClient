//
// Created by hohaia on 14/09/2026.
//

#ifndef WXCLIENT_TABLE_NAMES_H
#define WXCLIENT_TABLE_NAMES_H

#include <array>
#include <string>
#include <string_view>

namespace ict::core
{
    // A single entry from a controller table (recId: "0", label: "Entry Door" [door @ address 0]).
    struct RecordEntry
    {
        std::string recId;
        std::string label;
    };

    // Pairs a 'display name' with a controller's 'table name' (label: "Doors", name: "GXT_DOORS_TBL").
    struct TableInfo
    {
        std::string_view label;
        std::string_view name;
    };

    // An array of all of the controller tables (as a 'TableInfo' {label, name} pair).
    constexpr std::array allTables{
        TableInfo{"Access Levels", "GXT_ACCESSLEVELS_TBL"},
        TableInfo{"Analog Expanders", "GXT_ANALOGEXPANDERS_TBL"},
        TableInfo{"Areas", "GXT_AREAS_TBL"},
        TableInfo{"Area Groups", "GXT_AREAGROUPS_TBL"},
        TableInfo{"Automation", "GXT_AUTOMATION_TBL"},
        TableInfo{"Cameras", "GXT_CAMERAS_TBL"},
        TableInfo{"Controllers", "GXT_CONTROLLERS_TBL"},
        TableInfo{"Controller Settings", "GXT_CONTROLLERSETTINGS_TBL"},
        TableInfo{"Credential Types", "GXT_CREDENTIALTYPES_TBL"},
        TableInfo{"Data Values", "GXT_DATAVALUES_TBL"},
        TableInfo{"Daylight Savings", "GXT_DAYLIGHTSAVINGS_TBL"},
        TableInfo{"Doors", "GXT_DOORS_TBL"},
        TableInfo{"Door Groups", "GXT_DOORGROUPS_TBL"},
        TableInfo{"Door Types", "GXT_DOORTYPES_TBL"},
        TableInfo{"Elevator Cars", "GXT_ELEVATORCARS_TBL"},
        TableInfo{"Elevator Groups", "GXT_ELEVATORGROUPS_TBL"},
        TableInfo{"Event Reports", "GXT_EVENTREPORTS_TBL"},
        TableInfo{"Floor", "GXT_FLOORS_TBL"},
        TableInfo{"Floor Groups", "GXT_FLOORGROUPS_TBL"},
        TableInfo{"Holiday Group", "GXT_HOLIDAYGROUPS_TBL"},
        TableInfo{"Inputs", "GXT_INPUTS_TBL"},
        TableInfo{"Input Expanders", "GXT_INPUTEXPANDERS_TBL"},
        TableInfo{"Input Types", "GXT_INPUTTYPES_TBL"},
        TableInfo{"Keypads", "GXT_KEYPADS_TBL"},
        TableInfo{"Keypad Groups", "GXT_KEYPADGROUPS_TBL"},
        TableInfo{"Menu Groups", "GXT_MENUGROUPS_TBL"},
        TableInfo{"Outputs", "GXT_PGMS_TBL"},
        TableInfo{"Output Expanders", "GXT_PGMEXPANDERS_TBL"},
        TableInfo{"Output Groups", "GXT_PGMGROUPS_TBL"},
        TableInfo{"Phone Numbers", "GXT_PHONENUMBERS_TBL"},
        TableInfo{"Programmable Functions", "GXT_PROGRAMMABLEFUNCTIONS_TBL"},
        TableInfo{"Reader Expanders", "GXT_READEREXPANDERS_TBL"},
        TableInfo{"Schedules", "GXT_SCHEDULES_TBL"},
        TableInfo{"Services", "GXT_SERVICES_TBL"},
        TableInfo{"Site Details", "GXT_SITEDETAILS_TBL"},
        TableInfo{"Reader Units", "GXT_READERUNITS_TBL"},
        TableInfo{"Trouble Inputs", "GXT_TROUBLEINPUTS_TBL"},
        TableInfo{"Users", "GXT_USERS_TBL"}
    };
}

#endif //WXCLIENT_TABLE_NAMES_H
