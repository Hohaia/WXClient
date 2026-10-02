//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_INPUT_STATUS_TABLES_H
#define WXCLIENT_INPUT_STATUS_TABLES_H

#include <array>

#include "status_code_types.h"

namespace ict::core
{
    // 'Status 1 - Input Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array inputStatusCodes{
        StatusCodeLabel{0, "Closed/Off"},
        StatusCodeLabel{1, "Open/On"},
        StatusCodeLabel{2, "Tamper"},
        StatusCodeLabel{3, "Short Circuit"}
    };

    // 'Status 2 - Input Flags' (as a StatusCodeLabel {bit, label} pair).
    constexpr std::array inputFlagBits{
        StatusCodeLabel{0, "Bypassed"},
        StatusCodeLabel{1, "Bypassed (latched)"},
        StatusCodeLabel{2, "Siren Lockout"}
    };

    // Input status fields ("1,0" -> 1: "Open/On", 0: [No Flags]).
    constexpr std::array inputStatusFields{
        StatusCodeField{"Input", StatusType::Value, inputStatusCodes},
        StatusCodeField{"Flag(s)", StatusType::Bits, inputFlagBits}
    };
}

#endif //WXCLIENT_INPUT_STATUS_TABLES_H
