//
// Created by hohaia on 24/09/2026.
//

#ifndef WXCLIENT_STATUS_CODE_TABLES_H
#define WXCLIENT_STATUS_CODE_TABLES_H

#include <array>
#include <span>
#include <string_view>

namespace ict
{
    enum class StatusType { Value, Bits, Raw };

    struct StatusCodeLabel
    {
        int code; // Value: the code. Bits: the bit number.
        std::string_view text;
    };

    struct StatusCodeField
    {
        std::string_view name;
        StatusType type;
        std::span<const StatusCodeLabel> labels;
    };

    struct StatusCodeTable
    {
        std::string_view tableName;
        std::span<const StatusCodeField> fields;
    };

    // Doors.
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
    constexpr std::array doorPositionStatusCodes{
        StatusCodeLabel{0, "Closed"},
        StatusCodeLabel{1, "Open"},
        StatusCodeLabel{2, "Open Alert"},
        StatusCodeLabel{3, "Left Open"},
        StatusCodeLabel{4, "Forced Open"},
        StatusCodeLabel{5, "Bonding Fault"}
    };
    constexpr std::array doorFlagStatusCodes{
        StatusCodeLabel{0, "Calendar Action Live"}
    };
    constexpr std::array doorStatusFields{
        StatusCodeField{"Lock", StatusType::Value, doorLockStatusCodes},
        StatusCodeField{"Position", StatusType::Value, doorPositionStatusCodes},
        StatusCodeField{"Flag", StatusType::Bits, doorFlagStatusCodes}
    };

    // All tables.
    constexpr std::array statusCodeTables{
        // StatusCodeTable {"GXT_ANALOGEXPANDERS_TBL", xStatusFields},
        // StatusCodeTable {"GXT_AREAS_TBL", xStatusFields},
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
