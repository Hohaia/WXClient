//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_OUTPUT_STATUS_TABLES_H
#define WXCLIENT_OUTPUT_STATUS_TABLES_H

#include <array>

#include "status_code_types.h"

namespace ict::core
{
    // 'Status 1 - Output Status' (as a StatusCodeLabel {code, label} pair).
    constexpr std::array outputStatusCodes{
        StatusCodeLabel{0, "Off"},
        StatusCodeLabel{1, "On"},
        StatusCodeLabel{2, "Pulse On"},
        StatusCodeLabel{3, "On (Timed)"},
        StatusCodeLabel{4, "Pulse On (Timed)"},
        StatusCodeLabel{5, "Off by Conditional Exception"},
        StatusCodeLabel{6, "On by Conditional Exception"}
    };

    // 'Status 2 - Output Flags' (as a StatusCodeLabel {bit, label} pair).
    constexpr std::array outputFlagBits{
        StatusCodeLabel{0, "Calendar Action Active"}
    };

    // Output status fields ("1,0" -> 1: "On", 0: [No Flags]).
    constexpr std::array outputStatusFields{
        StatusCodeField{"Output", StatusType::Value, outputStatusCodes},
        StatusCodeField{"Flag(s)", StatusType::Bits, outputFlagBits}
    };
}

#endif //WXCLIENT_OUTPUT_STATUS_TABLES_H
