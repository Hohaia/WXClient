//
// Created by hohaia on 04/10/2026.
//

#ifndef WXCLIENT_INPUT_CONTROL_TABLES_H
#define WXCLIENT_INPUT_CONTROL_TABLES_H

#include <array>

#include "control_command_types.h"

namespace ict::core
{
    // 'Inputs' from 'control.md' (as a ControlCommand {code, label} pair).
    constexpr std::array inputControlCommands{
        ControlCommand{0, "Remove Bypass"},
        ControlCommand{1, "Bypass"},
        ControlCommand{2, "Bypass (latched)"}
    };
}

#endif //WXCLIENT_INPUT_CONTROL_TABLES_H
