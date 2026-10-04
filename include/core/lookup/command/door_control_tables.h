//
// Created by hohaia on 02/10/2026.
//

#ifndef WXCLIENT_DOOR_CONTROL_TABLES_H
#define WXCLIENT_DOOR_CONTROL_TABLES_H

#include <array>

#include "control_command_types.h"

namespace ict::core
{
    // 'Doors' from 'control.md' (as a ControlCommand {code, label} pair).
    constexpr std::array doorControlCommands{
        ControlCommand{0, "Lock"},
        ControlCommand{1, "Unlock"},
        ControlCommand{2, "Unlock (latched)"},
        ControlCommand{3, "Lockdown (allow entry)"},
        ControlCommand{4, "Lockdown (allow exit)"},
        ControlCommand{5, "Lockdown (allow entry and exit)"},
        ControlCommand{6, "Clear Lockdown"},
        ControlCommand{7, "Cancel Conditional Exception"},
        ControlCommand{8, "Restore Conditional Exception"},
        ControlCommand{9, "Lockdown (full)"},
        ControlCommand{11, "Reset Door Duress"}
    };
}

#endif //WXCLIENT_DOOR_CONTROL_TABLES_H
