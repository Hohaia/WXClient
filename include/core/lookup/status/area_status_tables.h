//
// Created by hohaia on 30/09/2026.
//

#ifndef WXCLIENT_AREA_STATUS_TABLES_H
#define WXCLIENT_AREA_STATUS_TABLES_H

#include <array>

#include "status_code_types.h"

namespace ict::core
{
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

    // 'Status 4 - User Count' (a raw int)

    // Area status fields ("0,128,3,0" -> 0: "24hr Disarmed", 128: "Armed", 3: "Alarm Activated, Siren Activated", 0: [No User Count]).
    constexpr std::array areaStatusFields{
        StatusCodeField{"24hr", StatusType::Value, area24HrStatusCodes},
        StatusCodeField{"Area", StatusType::Value, areaStatusCodes},
        StatusCodeField{"Notification(s)", StatusType::Bits, areaNotificationBits},
        StatusCodeField{"User Count", StatusType::Raw, {}}
    };
}

#endif //WXCLIENT_AREA_STATUS_TABLES_H
