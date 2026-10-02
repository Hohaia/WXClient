//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_TROUBLE_INPUT_STATUS_TABLES_H
#define WXCLIENT_TROUBLE_INPUT_STATUS_TABLES_H

#include <array>

#include "status_code_types.h"

namespace ict::core
{
    // 'Status 1 - Trouble Input Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array troubleInputStatusCodes{
        StatusCodeLabel{0, "Closed/Off"},
        StatusCodeLabel{1, "Open/On"}
    };

    // 'Status 2 - Trouble Input Flags' (as a StatusCodeLabel {bit, label} pair).
    constexpr std::array troubleInputFlagBits{
        StatusCodeLabel{0, "Bypassed"},
        StatusCodeLabel{1, "Bypassed (latched)"}
    };

    // Trouble input status fields ("1,0" -> 1: "Open/On", 0: [No Flags]).
    constexpr std::array troubleInputStatusFields{
        StatusCodeField{"Trouble Input", StatusType::Value, troubleInputStatusCodes},
        StatusCodeField{"Flag(s)", StatusType::Bits, troubleInputFlagBits}
    };
}

#endif //WXCLIENT_TROUBLE_INPUT_STATUS_TABLES_H
