//
// Created by hohaia on 30/09/2026.
//

#ifndef WXCLIENT_DOOR_STATUS_TABLES_H
#define WXCLIENT_DOOR_STATUS_TABLES_H

#include <array>

#include "status_code_types.h"

namespace ict::core
{
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
    constexpr std::array doorFlagBits{
        StatusCodeLabel{0, "Calendar Action Active"}
    };

    // Door status fields ("0,3,0" -> 0: "Locked", 3: "Left Open", 0: [No Flags]).
    constexpr std::array doorStatusFields{
        StatusCodeField{"Lock", StatusType::Value, doorLockStatusCodes},
        StatusCodeField{"Position", StatusType::Value, doorPositionStatusCodes},
        StatusCodeField{"Flag(s)", StatusType::Bits, doorFlagBits}
    };
}

#endif //WXCLIENT_DOOR_STATUS_TABLES_H
