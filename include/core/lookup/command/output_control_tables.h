//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_OUTPUT_CONTROL_TABLES_H
#define WXCLIENT_OUTPUT_CONTROL_TABLES_H

#include <array>

#include "control_command_types.h"

namespace ict::core
{
    // 'Outputs' from 'control.md' (as a ControlCommand {code, label} pair).
    constexpr std::array outputControlCommands{
        ControlCommand{0, "Deactivate"},
        ControlCommand{1, "Activate"},
        ControlCommand{2, "Activate (timed)", "Activation Time"}, // Data1 = "Activation Time".
        ControlCommand{3, "Cancel Conditional Exception"},
        ControlCommand{4, "Restore Conditional Exception"}
    };
}

#endif //WXCLIENT_OUTPUT_CONTROL_TABLES_H
