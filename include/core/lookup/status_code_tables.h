//
// Created by hohaia on 24/09/2026.
//

#ifndef WXCLIENT_STATUS_CODE_TABLES_H
#define WXCLIENT_STATUS_CODE_TABLES_H

#include <array>
#include <span>
#include <string_view>

namespace ict::core
{
    // How to decode a single field, e.g. for doorPositionStatus (Value: 3 -> "Left Open", Bits: 3 -> 011, Raw: 3 as is).
    enum class StatusType { Value, Bits, Raw };

    // A single status code and a label for it (code: 0, label: "Locked").
    struct StatusCodeLabel
    {
        int code; // Value: the code. Bits: the bit number.
        std::string_view label;
    };

    // One part of a record's status and how to decode it (name: "Lock", type: StatusType::Value, labels: doorLockStatusCodes).
    struct StatusCodeField
    {
        std::string_view name;
        StatusType type;
        std::span<const StatusCodeLabel> labels;
    };

    // All the status fields for one controller table (tableName: "GXT_DOORS_TBL", fields: doorStatusFields).
    struct StatusCodeTable
    {
        std::string_view name;
        std::span<const StatusCodeField> fields;
    };

    // Areas.
    // 'Status 1 - Area 24hr Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array area24HrStatusCodes{
        StatusCodeLabel{0, "24hr Disarmed"},
        StatusCodeLabel{1, "24hr Busy"},
        StatusCodeLabel{128, "24hr Armed"}
    };

    // 'Status 2 - Area Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array areaStatusCodes{
        StatusCodeLabel{0, "Disarmed"},
        StatusCodeLabel{1, "Zone(s) Open, Waiting for User"},
        StatusCodeLabel{2, "Trouble Condition, Waiting for User"},
        StatusCodeLabel{3, "Bypass Error, Waiting for User"},
        StatusCodeLabel{4, "Bypass Warning, Waiting for User"},
        StatusCodeLabel{5, "User Count not Zero, Waiting for User"},
        StatusCodeLabel{6, "Unknown"},
        StatusCodeLabel{127, "Unknown"},
        StatusCodeLabel{128, "Armed"},
        StatusCodeLabel{129, "Exit Delay"},
        StatusCodeLabel{130, "Entry Delay"},
        StatusCodeLabel{131, "Disarm Delay"},
        StatusCodeLabel{132, "Code Delay"}
    };

    // 'Status 3 - Area Notification Bits' (as a StatusCodeLabel {bit, label} pair).
    constexpr std::array areaNotificationBits{
        StatusCodeLabel{0, "Alarm Activated"},
        StatusCodeLabel{1, "Siren Activated"},
        StatusCodeLabel{2, "Alarms in Memory"},
        StatusCodeLabel{3, "Remote Armed"},
        StatusCodeLabel{4, "Force Armed"},
        StatusCodeLabel{5, "Instant Armed"},
        StatusCodeLabel{6, "Partial Armed"}
    };

    // Area status fields ("0,128,3,0" -> 0: "24hr Disarmed", 128: "Armed", 3: "Alarm Activated, Siren Activated", 0: [No User Count]).
    constexpr std::array areaStatusFields{
        StatusCodeField{"24hr", StatusType::Value, area24HrStatusCodes},
        StatusCodeField{"Area", StatusType::Value, areaStatusCodes},
        StatusCodeField{"Notification(s)", StatusType::Bits, areaNotificationBits},
        StatusCodeField{"User Count", StatusType::Raw, {}}
    };

    // Doors.
    // 'Status 1 - Door Lock Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array doorLockStatusCodes{
        StatusCodeLabel{0, "Locked"},
        StatusCodeLabel{1, "Unlocked by User"},
        StatusCodeLabel{2, "Unlocked by Schedule"},
        StatusCodeLabel{3, "Unlocked by User (timed)"},
        StatusCodeLabel{4, "Unlocked by User (latched)"},
        StatusCodeLabel{5, "Unlocked by REX"},
        StatusCodeLabel{6, "Unlocked by REN"},
        StatusCodeLabel{7, "Unlocked by Operator"},
        StatusCodeLabel{8, "Unlocked by Operator (timed)"},
        StatusCodeLabel{9, "Unlocked by Operator (latched)"},
        StatusCodeLabel{10, "Unlocked by Area"},
        StatusCodeLabel{11, "Unlocked by Fire Alarm"},
        StatusCodeLabel{12, "Locked by Conditional Exception"},
        StatusCodeLabel{13, "Unlocked by Conditional Exception"},
        StatusCodeLabel{14, "Unlocked by User using Extended Door Time"},
        StatusCodeLabel{15, "Unlocked by REX using Extended Door Time"},
        StatusCodeLabel{16, "Unlocked by REN using Extended Door Time"},
        StatusCodeLabel{17, "Unlocked by Operator using Extended Door Time"},
        StatusCodeLabel{18, "Locked using Extended Door Time"},
        StatusCodeLabel{19, "Locked Down (entry allowed)"},
        StatusCodeLabel{20, "Locked Down (exit allowed)"},
        StatusCodeLabel{21, "Locked Down (entry/exit allowed)"},
        StatusCodeLabel{22, "Locked Down (full lockdown)"},
        StatusCodeLabel{23, "Not Locked (locked, door not secure)"},
        StatusCodeLabel{24, "Not Locked Conditional (locked, door not secure, calendar action live)"}
    };

    // 'Status 2 - Door Position Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array doorPositionStatusCodes{
        StatusCodeLabel{0, "Closed"},
        StatusCodeLabel{1, "Open"},
        StatusCodeLabel{2, "Open Alert"},
        StatusCodeLabel{3, "Left Open"},
        StatusCodeLabel{4, "Forced Open"},
        StatusCodeLabel{5, "Bonding Fault"}
    };

    // 'Status 3 - Door Flag Bits' (as a StatusCodeLabel {bit, label} pair).
    constexpr std::array doorFlagStatusCodes{
        StatusCodeLabel{0, "Calendar Action Live"}
    };

    // Door status fields ("0,3,0" -> 0: "Locked", 3: "Left Open", 0: [No Flags]).
    constexpr std::array doorStatusFields{
        StatusCodeField{"Lock", StatusType::Value, doorLockStatusCodes},
        StatusCodeField{"Position", StatusType::Value, doorPositionStatusCodes},
        StatusCodeField{"Flag", StatusType::Bits, doorFlagStatusCodes}
    };

    // Status decoding for each controller table.
    constexpr std::array statusCodeTables{
        // StatusCodeTable {"GXT_ANALOGEXPANDERS_TBL", xStatusFields},
        StatusCodeTable {"GXT_AREAS_TBL", areaStatusFields},
        // StatusCodeTable {"GXT_CONTROLLERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_DATAVALUES_TBL", xStatusFields},
        StatusCodeTable{"GXT_DOORS_TBL", doorStatusFields},
        // StatusCodeTable {"GXT_FLOORS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_INPUTEXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_INPUTS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_KEYPADS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_PGMEXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_PGMS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_PROGRAMMABLEFUNCTIONS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_READEREXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_READERUNITS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_SCHEDULES_TBL", xStatusFields},
        // StatusCodeTable {"GXT_SERVICES_TBL", xStatusFields},
        // StatusCodeTable {"GXT_TROUBLEINPUTS_TBL", xStatusFields},
        // StatusCodeTable {"AREA_ZONE_STATUS", xStatusFields},
        // StatusCodeTable {"GXT_MODULES_TBL", xStatusFields},
        // StatusCodeTable {"ModuleFirmwareUpdate", xStatusFields},
    };
}

#endif //WXCLIENT_STATUS_CODE_TABLES_H
